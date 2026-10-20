#include <MotorControl.h>

MotorControl::MotorControl(int throttlePin, int steeringPin) {
    this->throttle.attach(throttlePin, 1000, 2000);
    this->steering.attach(steeringPin, 1000, 2000);

    this->throttle.setPeriodHertz(50);
    this->steering.setPeriodHertz(50);
}

int MotorControl::getNeutralSteeringValue() {
    return 1500 + this->steeringOffset;
}

void MotorControl::setToNeutral() {
    this->throttle.writeMicroseconds(1500 + this->throttleOffset);
    this->steering.writeMicroseconds(1500 + this->steeringOffset);
}

void MotorControl::forward() {
    this->throttle.writeMicroseconds(1620);
}

void MotorControl::backward() {
    this->throttle.writeMicroseconds(1300);
}

void MotorControl::brake() {
    this->throttle.writeMicroseconds(1500 + this->throttleOffset);
    this->throttle.writeMicroseconds(1400);

    delay(1000);

    this->throttle.writeMicroseconds(1500 + this->throttleOffset);
}

void MotorControl::steeringRight() {
    int steeringVal = 1950;

    if (this->steeringOffset < 0) {
        steeringVal += 2*this->steeringOffset;
    }

    this->steering.writeMicroseconds(steeringVal);
}

void MotorControl::steeringLeft() {
    int steeringVal = 1050;

    if (this->steeringOffset > 0) {
        steeringVal += 2*this->steeringOffset;
    }

    this->steering.writeMicroseconds(steeringVal);
}

void MotorControl::steeringNeutral() {
    this->steering.writeMicroseconds(1500 + this->steeringOffset);
}

void MotorControl::setThrottleVal(int bruteNewThrottle) {
    int toLow = 1000, toHigh = 2000;

    if (this->throttleOffset < 0) {
        toHigh = 2000 + 2*this->throttleOffset;
    } else {
        toLow = 1000 + 2*this->throttleOffset;
    }

    int newThrottle = map(bruteNewThrottle, 1000, 2000, toLow, toHigh);

    this->throttle.writeMicroseconds(newThrottle);
}

void MotorControl::setSteeringVal(int bruteNewSteering) {
    int toLow = 1000, toHigh = 2000;

    if (this->steeringOffset < 0) {
        toHigh = 2000 + 2*this->steeringOffset;
    } else {
        toLow = 1000 + 2*this->steeringOffset;
    }

    int newSteering = map(bruteNewSteering, 1000, 2000, toLow, toHigh);

    this->steering.writeMicroseconds(newSteering);
}

void MotorControl::updateThrottleOffset(bool positive) {
    if (positive) {
        if (this->throttleOffset < 200) {
            this->throttleOffset += 10;
        }
    } else {
        if (this->throttleOffset > -200) {
            this->throttleOffset -= 10;
        }
    }
}

void MotorControl::updateSteeringOffset(bool positive) {
    if (positive) {
        if (this->steeringOffset < 200) {
            this->steeringOffset += 10;
        }
    } else {
        if (this->steeringOffset > -200) {
            this->steeringOffset -= 10;
        }
    }
}