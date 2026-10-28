#include <Scanner.h>

Scanner::Scanner(DistanceSensor& sensorRef, DistanceFilter& filterRef, SensorServo& servoRef):
    sensor(sensorRef),
    filter(filterRef),
    servo(servoRef)

{
}

void Scanner::startScanner() {
    this->sensor.begin();
    this->servo.moveSensor(SensorDirections::MID);
}

bool Scanner::update() {
    unsigned long now = millis();

    if (this->distance < 0) {
        float rawDistance = this->sensor.getDistance() / 10;
        if (rawDistance < 0) {
            return true;
        }

        this->distance = this->filter.filterMeasurement(rawDistance);
        this->lastSampleTime = now;
        return true;
    }

    if (now - this->lastSampleTime >= this->sampleTimePeriod) {
        float rawDistance = this->sensor.getDistance() / 10;
        if (rawDistance < 0) {
            return true;
        }
        
        if (rawDistance - this->distance < this->MAX_JUMP) {
            this->suspiciousCount = 0;
            this->distance = this->filter.filterMeasurement(rawDistance);
            return true;
        } else {
            this->suspiciousCount++;

            if (this->suspiciousCount >= 2) {
                this->distance = this->filter.filterMeasurement(rawDistance);
                this->suspiciousCount = 0;
                return true;
            }
        }
        
        this->lastSampleTime = now;
    }
    return false;
}

float Scanner::getFilteredDistance() {
    return this->distance;
}

void Scanner::resetSensorFilter() {
    this->filter.resetFilter();
}

float Scanner::getMedian() {
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

bool Scanner::scanning() {
    bool rightLeftDistancesReady = false;
    unsigned long now = millis();

    switch (this->scanningState) {
        case ScanningState::WAITING:
            this->sampleIndex = 0;
            this->leftDistance = -1;
            this->rightDistance = -1;

            this->movingStartTime = millis();
            this->servo.moveSensor(SensorDirections::LEFT);
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

                this->values[this->sampleIndex] = this->sensor.getDistance();
                if (this->values[this->sampleIndex] >= 0) {
                    this->sampleIndex++;
                }

                if (this->sampleIndex >= 3) {
                    this->leftDistance = this->getMedian();
                    this->sampleIndex = 0;

                    this->movingStartTime = millis();
                    this->servo.moveSensor(SensorDirections::RIGHT);
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

                this->values[this->sampleIndex] = this->sensor.getDistance();
                if (this->values[this->sampleIndex] >= 0) {
                    this->sampleIndex++;
                }

                if (this->sampleIndex >= 3) {
                    this->rightDistance = this->getMedian();
                    this->sampleIndex = 0;

                    this->movingStartTime = millis();
                    this->servo.moveSensor(SensorDirections::MID);
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

void Scanner::resetScanning() {
    this->scanningState = ScanningState::WAITING;
    this->sampleIndex = 0;
    this->leftDistance = -1;
    this->rightDistance = -1;
    this->servo.moveSensor(SensorDirections::MID);
}

float Scanner::getRightDistance() {
    float dist = this->rightDistance;
    this->rightDistance = -1;

    return dist;
}

float Scanner::getLeftDistance() {
    float dist = this->leftDistance;
    this->leftDistance = -1;

    return dist;
}