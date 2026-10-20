#include <WebInterface.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Utility.cpp>

WebInterface::WebInterface():
    server(80),
    webSocket(1222)

{   
}

void WebInterface::webSocketsEvents(uint8_t clientNum, WStype_t type, uint8_t* payload, size_t length) {
    RobotCommands command{};
    command.source = CommandSource::WEB_INTERFACE;
    command.scope = CommandScope::MANUAL;
    command.type = CommandType::NONE;

    if (type == WStype_CONNECTED) {
        if (this->activeUser == -1) {
            this->activeUser = clientNum;
            command.type = CommandType::CONNECTED;
            command.scope = CommandScope::BOTH;
        } else {
            this->webSocket.disconnect(clientNum);
            command.type = CommandType::CONNECTION_REJECTED;
            command.scope = CommandScope::BOTH;
        }
    } else if (type == WStype_TEXT) {
        if (clientNum != this-> activeUser) {
            return;
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        
        if (error) {
            char buff[256];
            snprintf(buff, sizeof(buff), 
                "{\"event\":\"deserializationError\",\"error\":\"%s\"}",
                error.c_str()
            );

            this->webSocket.sendTXT(clientNum, buff);

            return;
        }
        
        const char* event = doc["event"].as<const char*>();

        if (event == nullptr) {
            return;
        }

        if (strcmp(event, "powerButton") == 0) {
            command.type = CommandType::POWERBUTTON;
            command.scope = CommandScope::BOTH;
            /* sendStateMessage(); */
        } else if (strcmp(event, "signalData") == 0) {
            if (!doc["throttle"].is<int>() || !doc["steering"].is<int>()) {
                return;
            }

            int bruteNewThrottle = doc["throttle"].as<int>() * 10 + 1000;
            int bruteNewSteering = doc["steering"].as<int>() * 10 + 1000;

            if (bruteNewThrottle < 1000 || bruteNewThrottle > 2000 || bruteNewSteering < 1000 || bruteNewSteering > 2000) {
                return;
            }

            command.type = CommandType::MANUAL_DRIVE;
            command.throttleVal = bruteNewThrottle;
            command.steeringVal = bruteNewSteering;
        } else if (strcmp(event, "leftTrimm") == 0) {
            command.type = CommandType::STEERING_TRIMMER;
            command.steeringTrimmPositive = false;

            /* sendNeutralValue(clientNum); */
        } else if (strcmp(event, "rightTrimm") == 0) {
            command.type = CommandType::STEERING_TRIMMER;
            command.steeringTrimmPositive = true;

            /* sendNeutralValue(clientNum); */
        } else if (strcmp(event, "requestModeChange") == 0) {
            command.type = CommandType::CHANGE_MODE;
            command.scope = CommandScope::BOTH;
        }
            
    } else if (type == WStype_DISCONNECTED) {
        if (this->activeUser == clientNum) {
            this->activeUser = -1;
            command.type = CommandType::DISCONNECTED;
            command.scope = CommandScope::BOTH;
        }
    }

    if (command.type != CommandType::NONE) {
        if (command.type == CommandType::MANUAL_DRIVE) {
            this->latestDriveCommand = command;
            this->driveCommandReady = true;
        } else {
            this->pendingCommands.push(command);
        }
    }

    
}

void WebInterface::initializeWebConnection() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS error");
        return;
    }

    this->server.on("/", HTTP_GET, [this]() {
        File file = LittleFS.open("/index.html", "r");

        this->server.streamFile(
            file,
            "text/html"
        );

        file.close();
    });

    this->server.serveStatic(
        "/",
        LittleFS,
        "/"
    );

    this->server.begin();

    this->webSocket.onEvent(
        [this](uint8_t clientNum, 
                WStype_t type, 
                uint8_t* payload, 
                size_t length) {
                    this->webSocketsEvents(clientNum, type, payload, length);
                }
    );
    this->webSocket.begin();
}

void WebInterface::update() {
    constexpr int MAX_WS_LOOPS = 8;

    for (int i = 0; i < MAX_WS_LOOPS; i++) {
        this->webSocket.loop();
    }

    this->server.handleClient();
}

const bool WebInterface::hasCommands() {
    return !this->pendingCommands.empty();
}

RobotCommands WebInterface::popPendingCommand(){
    RobotCommands command = this->pendingCommands.front();
    this->pendingCommands.pop();

    return command;
}

const bool WebInterface::isDriveCommandReady() {
    return this->driveCommandReady;
}

RobotCommands WebInterface::getLatestDriveCommand() {
    this->driveCommandReady = false;

    return this->latestDriveCommand;
}

void WebInterface::sendAutonomousState(AutonomousState state) {
    if (this->activeUser == -1) {
        return;
    }

    char buff[256];
    snprintf(buff, sizeof(buff), 
        "{\"event\":\"autonomousState\",\"state\":\"%s\"}",
        autonomousStateToString(state).c_str()
    );

    this->webSocket.sendTXT(this->activeUser, buff);
}

void WebInterface::sendRobotMode(RobotMode mode) {
    if (this->activeUser == -1 || mode == RobotMode::SHUTDOWN) {
        return;
    }

    char buff[256];

    snprintf(buff, sizeof(buff), 
        "{\"event\":\"changeMode\",\"mode\":\"%s\"}",
        robotModeToString(mode).c_str()
    );

    this->webSocket.sendTXT(this->activeUser, buff);
}

void WebInterface::sendStateLightMode(bool on) {
    if (this->activeUser == -1) {
        return;
    }

    char buff[256];

    snprintf(buff, sizeof(buff), 
        "{\"event\":\"stateLightChange\",\"state\":%s}",
        on ? "true" : "false"
    );

    this->webSocket.sendTXT(this->activeUser, buff);
}

void WebInterface::sendSteeringNeutralValue(int val) {
    if (this->activeUser == -1) {
        return;
    }

    char buff[256];

    snprintf(buff, sizeof(buff), 
        "{\"event\":\"changeNeutralValue\",\"value\":%d}",
        val
    );

    this->webSocket.sendTXT(this->activeUser, buff);
}

void WebInterface::sendDistance(float distance) {
    if (this->activeUser == -1) {
        return;
    }

    char buff[256];

    if (distance >= 0) {
        snprintf(buff, sizeof(buff), 
            "{\"event\":\"sensorData\",\"distance\":%.2f}",
            distance
        );
    } else {
        snprintf(buff, sizeof(buff), 
            "{\"event\":\"sensorData\",\"distance\":null}"
        );
    }

    this->webSocket.sendTXT(this->activeUser, buff);
}

void WebInterface::sendMessage(std::string event, std::string msg) {
    char buff[256];

    snprintf(buff, sizeof(buff), 
        "{\"event\": \"%s\",\"message\": \"%s\"}",
        event, msg
    );

    this->webSocket.sendTXT(this->activeUser, buff);
}