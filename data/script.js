const ws = new WebSocket("ws://" + location.hostname + ":1222/");

/* Throttle and steering */

const throttleCursor = document.getElementById("throttleCursor");
const steeringCursor = document.getElementById("steeringCursor");

let throttleValue = 50;
let steeringValue = 50;

let lastThrottleValue = null;
let lastSteeringValue = null;

throttleCursor.addEventListener("input", function() {
    throttleValue = Number(throttleCursor.value);
});

steeringCursor.addEventListener("input", function() {
    steeringValue = Number(steeringCursor.value);
});

throttleCursor.addEventListener("pointerup", function() {
    const minThrottle = Number(throttleCursor.min);
    const maxThrottle = Number(throttleCursor.max);

    throttleCursor.value = (minThrottle + maxThrottle) / 2;

    throttleValue = Number(throttleCursor.value);
});

steeringCursor.addEventListener("pointerup", function() {
    const minsteering = Number(steeringCursor.min);
    const maxsteering = Number(steeringCursor.max);

    steeringCursor.value = (minsteering + maxsteering) / 2;

    steeringValue = Number(steeringCursor.value);
});

const sendValues = () => {
    if (ws.readyState !== WebSocket.OPEN) return;
    
    if (throttleValue !== lastThrottleValue || steeringValue !== lastSteeringValue) {
        const msg = {
            event: "signalData",
            throttle: throttleValue,
            steering: steeringValue
        
        };

        lastThrottleValue = throttleValue;
        lastSteeringValue = steeringValue;

        ws.send(JSON.stringify(msg));
    }
}

setInterval(sendValues, 40);

/* BUTTONS */

/* On/Off button */

const powerButton = document.getElementById("powerButton");
const stateLight = document.getElementById("stateLight");

powerButton.addEventListener("click", function() {
    const msg = {
        event: "powerButton"
    };

    ws.send(JSON.stringify(msg));
});

/*  Write and download data file. Download button */

let data = "time,distance\n";

const addMeasurementsToFile = (time, distance) => {
    data += `${time},${distance}\n`;
}

const downloadFile = () => {
    const blob = new Blob([data], {type: "text/csv"});
    const url = URL.createObjectURL(blob);

    const a = document.createElement('a');
    a.href = url;
    a.download = "measurements.csv";
    a.click();

    URL.revokeObjectURL(blob);
}

const downloadButton = document.getElementById("downloadButton");

downloadButton.addEventListener("click", () => {
    downloadFile();
})

/* Change mode button */

const changeModeButton = document.getElementById("changeModeButton");
let lastChangeModeTime = 0;

const requestModeChange = () => {
    const now = performance.now();

    if (now - lastChangeModeTime < 500) {
        return;
    }
    lastChangeModeTime = now;

    const msg = {
        event: "requestModeChange"
    };

    ws.send(JSON.stringify(msg));
}

changeModeButton.addEventListener("click", () => {
    requestModeChange();
});

/* Trimmer */

const leftArrow = document.getElementById("leftArrow");
const rightArrow = document.getElementById("rightArrow");

leftArrow.addEventListener("click", () => {
    const msg = {
        event: "leftTrimm"
    };

    ws.send(JSON.stringify(msg));
});

rightArrow.addEventListener("click", () => {
    const msg = {
        event: "rightTrimm"
    };

    ws.send(JSON.stringify(msg));
});

const changeNeutralValue = (val) => {
    neutralValueDisplay.textContent = String(val);
}

/* Graph */

const canvas = document.getElementById("distanceGraph");
const ctx = canvas.getContext("2d");

const margin = {
    left: 45,
    right: 10,
    top: 10,
    bottom: 30
};

const maxDistance = 300;
const timeWindow = 20;

let graphWidth = canvas.width - margin.left - margin.right;
let graphHeight = canvas.height - margin.top - margin.bottom;

const distanceToY = (distance) => {
    return canvas.height - margin.bottom - (distance / maxDistance) * graphHeight;
}

const timeToX = (time) => {
    return margin.left + (time / timeWindow) * graphWidth;
}

let measurements = [];

const clearGraph = () => {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
}

const drawAxes = () => {
    ctx.beginPath();

    // axa Y
    ctx.moveTo(
        margin.left,
        margin.top
    );

    ctx.lineTo(
        margin.left,
        canvas.height - margin.bottom
    );

    // axa X
    ctx.lineTo(
        canvas.width - margin.right,
        canvas.height - margin.bottom
    );

    ctx.strokeStyle = "black";
    ctx.lineWidth = 2;
    ctx.stroke();
}

const drawGrid = () => {

    // -------------------------
    // Y
    // -------------------------

    ctx.font = "11px Arial";
    ctx.textAlign = "right";
    ctx.textBaseline = "middle";

    for (let distance = 0; distance <= maxDistance; distance += 50) {

        const y = distanceToY(distance);

        // grid orizontal
        ctx.beginPath();
        ctx.moveTo(margin.left, y);
        ctx.lineTo(
        canvas.width - margin.right,
        y
        );

        ctx.strokeStyle = "#dddddd";
        ctx.lineWidth = 1;
        ctx.stroke();

        // gradatie Y
        ctx.beginPath();
        ctx.moveTo(
        margin.left - 5,
        y
        );

        ctx.lineTo(
        margin.left,
        y
        );

        ctx.strokeStyle = "black";
        ctx.stroke();

        // text
        ctx.fillStyle = "black";

        ctx.fillText(
        distance,
        margin.left - 8,
        y
        );
    }


    // -------------------------
    // X
    // -------------------------

    ctx.textAlign = "center";
    ctx.textBaseline = "top";

    for (let time = 0; time <= timeWindow; time += 5) {

        const x = timeToX(time);

        // grid vertical
        ctx.beginPath();
        ctx.moveTo(
        x,
        margin.top
        );

        ctx.lineTo(
        x,
        canvas.height - margin.bottom
        );

        ctx.strokeStyle = "#eeeeee";
        ctx.stroke();

        // gradatie X
        ctx.beginPath();

        ctx.moveTo(
        x,
        canvas.height - margin.bottom
        );

        ctx.lineTo(
        x,
        canvas.height - margin.bottom + 5
        );

        ctx.strokeStyle = "black";
        ctx.stroke();

        // text
        ctx.fillStyle = "black";

        ctx.fillText(
        time + " s",
        x,
        canvas.height - margin.bottom + 8
        );
    }
}

const drawMeasurements = () => {

    if (measurements.length < 2) {
        return;
    }

    const latestTime =
        measurements[measurements.length - 1].time;

    const startTime =
        Math.max(0, latestTime - timeWindow);

    ctx.beginPath();

    let drawing = false;

    for (const measurement of measurements) {

        if (measurement.distance === null) {
            drawing = false;
            continue;
        }


        if (measurement.time < startTime) {
        continue;
        }

        const relativeTime =
        measurement.time - startTime;

        const x = timeToX(relativeTime);

        let distance = measurement.distance;

        distance = Math.max(
        0,
        Math.min(distance, maxDistance)
        );

        const y = distanceToY(distance);

        if (!drawing) {
        ctx.moveTo(x, y);
        drawing = true;
        } else {
        ctx.lineTo(x, y);
        }
    }

    ctx.strokeStyle = "blue";
    ctx.lineWidth = 2;
    ctx.stroke();
}

const drawGraph = () => {
    clearGraph();
    drawAxes();
    drawGrid();
    drawMeasurements();
}

drawGraph();

let lastGraphUpdate = 0;

const updateGraph = () => {
    const now = performance.now();

    if (now - lastGraphUpdate >= 50) {
        drawGraph();
        lastGraphUpdate = now;
    }
}

/* Change state light */

const changeStateLight = (state) => {
    if (state) {
        stateLight.classList.remove("off");
        stateLight.classList.add("on");

        document.documentElement.requestFullscreen();
    } else {
        stateLight.classList.remove("on");
        stateLight.classList.add("off");

        document.exitFullscreen();
    } 
}

/* Change Mode */

const stateDisplay = document.getElementById("stateDisplay");
const neutralValueDisplay = document.getElementById("neutralValueDisplay");
const graphWrapper = document.getElementById("graphWrapper");

let currentAutonomousState = "WAITING_FOR_STATE";
const stateScreen = document.getElementById("stateScreen");

const changeMode = (mode) => {
    if (mode === "AUTONOMOUS") {
        steeringCursor.classList.add("none");
        throttleCursor.classList.add("none");

        leftArrow.classList.add("none");
        rightArrow.classList.add("none");
        neutralValueDisplay.classList.add("hidden");

        stateDisplay.classList.remove("none");

        graphWrapper.classList.remove("gap");
        canvas.classList.remove("manual-mode-size");
        canvas.classList.add("auto-mode-size");
        drawGraph();
    } else if (mode === "MANUAL") {
        steeringCursor.classList.remove("none");
        throttleCursor.classList.remove("none");

        leftArrow.classList.remove("none");
        rightArrow.classList.remove("none");
        neutralValueDisplay.classList.remove("hidden");

        stateDisplay.classList.add("none");
        stateScreen.classList.remove(currentAutonomousState);
        currentAutonomousState = "WAITING_FOR_STATE";
        stateScreen.classList.add(currentAutonomousState);
        stateScreen.innerHTML = currentAutonomousState.replaceAll("_", "_<wbr>");

        graphWrapper.classList.add("gap");
        canvas.classList.remove("auto-mode-size");
        canvas.classList.add("manual-mode-size");
        drawGraph();
    } else {
        console.err("Unknown mode in changeMode()");
    }
}

/* Handle sensorData webSocket event */

const sensorData = (distance) => {
    const time = performance.now() / 1000;
        
        measurements.push({
            time,
            distance
        });

        if (measurements.length > 500) {
            measurements.shift();
        }
        
        addMeasurementsToFile(time, distance);
        updateGraph();
}

/* Afisarea starii autonome a robotului */

const displayAutonomousState = (state) => {
    stateScreen.classList.remove(currentAutonomousState);
    stateScreen.classList.add(state);

    currentAutonomousState = state;

    stateScreen.innerHTML = state.replaceAll("_", "_<wbr>");
}

const displayThrottleSteering = (throttle, steering) => {
    console.log("Throttle: ", throttle);
    console.log("Steering: ", steering);
}

const displayMessage = (msg) => {
    console.log(msg);
}

/* WebSocket */

ws.onmessage = (event) => {
    const msg = JSON.parse(event.data);

    if (msg.event === "stateLightChange") {
        changeStateLight(msg.state);
    } else if (msg.event === "changeNeutralValue") {
        changeNeutralValue(msg.value);
    } else if (msg.event === "sensorData") {
        sensorData(msg.distance);
    } else if (msg.event === "autonomousState") {
        displayAutonomousState(msg.state);
    } else if (msg.event === "changeMode") {
        changeMode(msg.mode);
    } else if (msg.event === "sentThrottleSteering") {
        displayThrottleSteering(msg.throttle, msg.steering);
    } else if (msg.event === "signalDataReceived") {
        displayMessage(msg.message);
    }
}