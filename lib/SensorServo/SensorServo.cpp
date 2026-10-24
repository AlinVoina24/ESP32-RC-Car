#include <SensorServo.h>

SensorServo::SensorServo(int servoPin) {
    this->sensorServo.attach(servoPin, 500, 2500);
    this->sensorServo.setPeriodHertz(50);
}

std::string SensorServo::dirToString(SensorDirections dir) {
    if (dir == SensorDirections::MID) {
        return "MID";
    }
    if (dir == SensorDirections::RIGHT) {
        return "RIGHT";
    }
    if (dir == SensorDirections::LEFT) {
        return "LEFT";
    }

    return "UNKNOWN";
}

void SensorServo::moveSensor(SensorDirections dir) {
    int toLow = 0, toHigh = 180;

    if (this->servoOffset < 0) {
        toHigh = 180 + 2*this->servoOffset;
    } else {
        toLow = 0 + 2*this->servoOffset;
    }

    if (this->dirs.find(dir) == this->dirs.end()) {
        char buff[128];

        snprintf(
            buff,
            sizeof(buff),
            "{\"event\":\"error\",\"message\":\"Invalid direction: %s\"}",
            this->dirToString(dir).c_str()
        );

        /* webSocket.broadcastTXT(buff); */

        return;
    }

    int normalDir = this->dirs[dir];
    int newSensorServo = map(normalDir, 0, 180, toLow, toHigh);

    this->sensorServo.write(newSensorServo);
}

void SensorServo::updateServoOffset(bool positive) {
    if (positive && this->servoOffset < 85) {
        this->servoOffset++;
    }

    if (!positive && this->servoOffset > -85) {
        this->servoOffset--;
    }
}

int SensorServo::getServoOffset() {
    return this->servoOffset;
}