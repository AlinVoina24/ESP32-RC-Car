#include <UltraSounds.h>

UltraSounds::UltraSounds(int trigPin, int echoPin, int servoPin) {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    this->trigPin = trigPin;
    this->echoPin = echoPin;

    this->sensorServo.attach(servoPin, 500, 2500);
    this->sensorServo.setPeriodHertz(50);

}

float UltraSounds::getDistance() {
    digitalWrite(this->trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(this->trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(this->trigPin, LOW);

    unsigned long duration = pulseIn(this->echoPin, HIGH, 20000);

    if (duration != 0) {
        return duration * 0.0343 / 2.0;
    }

    return 300; 
}

float UltraSounds::getMedianDistance() {
    const int N = 5;
    float values[N];

    for (int i = 0; i < N; i++) {
        values[i] = getDistance();

        if (i < N - 1) {
        delay(50);
        }
        
    }

    for (int i = 0; i < N - 1; i++) {
        int minIndex = i;

        for (int j = i + 1; j < N; j++) {
        if (values[j] < values[minIndex]) {
            minIndex = j;
        }
        }

        if (minIndex != i) {
        float temp = values[i];
        values[i] = values[minIndex];
        values[minIndex] = temp;
        }
    }

    return values[N / 2];
}

float UltraSounds::getMedian() {
    int N = 3;

    for (int i = 0; i < N - 1; i++) {
        int minIndex = i;

        for (int j = i + 1; j < N; j++) {
            if (this->values[j] < this->values[minIndex]) {
            minIndex = j;
            }
        }

        if (minIndex != i) {
            float temp = this->values[i];
            this->values[i] = this->values[minIndex];
            this->values[minIndex] = temp;
        }
    }

    return this->values[N / 2];
}

bool UltraSounds::update() {
    unsigned long now = millis();

    if (this->distance < 0) {
        float rawDistance = this->getDistance();
        this->distance = this->kf.filterMeasurement(rawDistance);
        this->lastSampleTime = now;
        return true;
    }

    if (now - this->lastSampleTime >= this->sampleTimePeriod) {
        float rawDistance = this->getDistance();
        
        if (rawDistance - this->distance < this->MAX_JUMP) {
            this->suspiciousCount = 0;
            this->distance = this->kf.filterMeasurement(rawDistance);
            return true;
        } else {
            this->suspiciousCount++;

            if (this->suspiciousCount >= 2) {
                this->distance = this->kf.filterMeasurement(rawDistance);
                this->suspiciousCount = 0;
                return true;
            }
        }
        
        this->lastSampleTime = now;
    }
    return false;
}

float UltraSounds::getFilteredDistance() {
    return this->distance;
}

void UltraSounds::moveSensor(SensorDirections dir) {
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

void UltraSounds::updateServoOffset(bool positive) {
    if (positive) {
        if (this->servoOffset < 15) {
            this->servoOffset += 5;
        }
    } else {
        if (this->servoOffset > -15) {
            this->servoOffset -= 5;
        }
    }
}

std::string UltraSounds::dirToString(SensorDirections dir) {
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

bool UltraSounds::scanning() {
    bool rightLeftDistancesReady = false;
    unsigned long now = millis();

    switch (this->scanningState) {
        case ScanningState::WAITING:
            this->sampleIndex = 0;
            this->leftDistance = -1;
            this->rightDistance = -1;

            this->movingStartTime = millis();
            this->moveSensor(SensorDirections::LEFT);
            this->scanningState = ScanningState::MOVING_LEFT;
            break;
        
        case ScanningState::MOVING_LEFT:
            if (now - this->movingStartTime >= 500) {
                this->scanningState = ScanningState::SCANNING_LEFT;
                this->lastSampleTimeRL = millis() - this->sampleTimePeriod;
            }
            
            break;

        case ScanningState::SCANNING_LEFT:
            if (now - this->lastSampleTimeRL >= this->sampleTimePeriod) {
                this->lastSampleTimeRL = now;

                this->values[this->sampleIndex] = this->getDistance();
                if (this->values[this->sampleIndex] >= 0) {
                    this->sampleIndex++;
                }

                if (this->sampleIndex >= 3) {
                    this->leftDistance = this->getMedian();
                    this->sampleIndex = 0;

                    this->movingStartTime = millis();
                    this->moveSensor(SensorDirections::RIGHT);
                    this->scanningState = ScanningState::MOVING_RIGHT;
                    break;
                }
            }

            break;

        case ScanningState::MOVING_RIGHT:
            if (now - this->movingStartTime >= 500) {
                this->scanningState = ScanningState::SCANNING_RIGHT;
                this->lastSampleTimeRL = millis() - this->sampleTimePeriod;
            }
            
            break;
        
        case ScanningState::SCANNING_RIGHT:
            if (now - this->lastSampleTimeRL >= this->sampleTimePeriod) {
                this->lastSampleTimeRL = now;

                this->values[this->sampleIndex] = this->getDistance();
                if (this->values[this->sampleIndex] >= 0) {
                    this->sampleIndex++;
                }

                if (this->sampleIndex >= 3) {
                    this->rightDistance = this->getMedian();
                    this->sampleIndex = 0;

                    this->movingStartTime = millis();
                    this->moveSensor(SensorDirections::MID);
                    this->scanningState = ScanningState::MOVING_MID;
                    break;
                }
            }

            break;
        
        case ScanningState::MOVING_MID:
            if (now - this->movingStartTime >= 500) {
                this->scanningState = ScanningState::READY;
            }
            
            break;
        
        case ScanningState::READY:
            rightLeftDistancesReady = true;
            this->scanningState = ScanningState::WAITING;
            break;

    }

    return rightLeftDistancesReady;
        
}

void UltraSounds::resetScanning() {
    this->scanningState = ScanningState::WAITING;
    this->sampleIndex = 0;
    this->leftDistance = -1;
    this->rightDistance = -1;
    this->moveSensor(SensorDirections::MID);
}

float UltraSounds::getRightDistance() {
    float dist = this->rightDistance;
    this->rightDistance = -1;

    return dist;
}

float UltraSounds::getLeftDistance() {
    float dist = this->leftDistance;
    this->leftDistance = -1;

    return dist;
}

void UltraSounds::resetKalmanFilter() {
    this->kf.reset();
}