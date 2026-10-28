#include <MotorControl.h>

MotorControl::MotorControl(int throttlePin, int steeringPin) {
    this->throttle.attach(throttlePin, 1000, 2000);
    this->steering.attach(steeringPin, 1000, 2000);

    this->throttle.setPeriodHertz(50);
    this->steering.setPeriodHertz(50);
}

int MotorControl::normaliseThrottleVal(int throttleVal) {
    int toLow = 1000, toHigh = 2000;

    if (this->throttleOffset < 0) {
        toHigh = 2000 + 2*this->throttleOffset;
    } else {
        toLow = 1000 + 2*this->throttleOffset;
    }

    int normalisedThrottleVal = map(throttleVal, 1000, 2000, toLow, toHigh);

    return normalisedThrottleVal;
}

int MotorControl::normaliseSteeringVal(int steeringVal) {
    int toLow = 1000, toHigh = 2000;

    if (this->steeringOffset < 0) {
        toHigh = 2000 + 2*this->steeringOffset;
    } else {
        toLow = 1000 + 2*this->steeringOffset;
    }

    int normalisedSteeringVal = map(steeringVal, 1000, 2000, toLow, toHigh);

    return normalisedSteeringVal;
}

int MotorControl::getNeutralSteeringValue() {
    return 1500 + this->steeringOffset;
}

void MotorControl::setToNeutral() {
    this->throttleVal = 1500 + this->throttleOffset;
    this->steeringVal = 1500 + this->steeringOffset;

    this->throttle.writeMicroseconds(this->throttleVal);
    this->steering.writeMicroseconds(this->steeringVal);
}

void MotorControl::forward() {
    this->throttleVal = this->normaliseThrottleVal(1650);
    this->throttle.writeMicroseconds(this->throttleVal);
}

void MotorControl::backward() {
    this->throttleVal = this->normaliseThrottleVal(1300);
    this->throttle.writeMicroseconds(this->throttleVal);
}

void MotorControl::brake() {
    this->throttleVal = 1500 + this->throttleOffset;
    this->throttle.writeMicroseconds(this->throttleVal);
}

void MotorControl::steeringRight() {
    this->steeringVal = this->normaliseSteeringVal(1950);
    this->steering.writeMicroseconds(this->steeringVal);
}

void MotorControl::steeringLeft() {
    this->steeringVal = this->normaliseSteeringVal(1050);
    this->steering.writeMicroseconds(this->steeringVal);
}

void MotorControl::steeringNeutral() {
    this->steering.writeMicroseconds(1500 + this->steeringOffset);
}

void MotorControl::setThrottleVal(int bruteNewThrottle) {
    this->throttleVal = this->normaliseThrottleVal(bruteNewThrottle);
    this->throttle.writeMicroseconds(this->throttleVal);
}

void MotorControl::setSteeringVal(int bruteNewSteering) {
    this->steeringVal = this->normaliseSteeringVal(bruteNewSteering);
    this->steering.writeMicroseconds(this->steeringVal);
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