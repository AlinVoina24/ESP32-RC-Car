#include <UltrasonicSensor.h>

UltrasonicSensor::UltrasonicSensor(int trigPin, int echoPin) {
    this->trigPin = trigPin;
    this->echoPin = echoPin;
}

bool UltrasonicSensor::begin() {
    pinMode(this->trigPin, OUTPUT);
    pinMode(this->echoPin, INPUT);

    return true;
}

float UltrasonicSensor::getDistance() {
    digitalWrite(this->trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(this->trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(this->trigPin, LOW);

    unsigned long duration = pulseIn(this->echoPin, HIGH, 20000);

    if (duration != 0) {
        return duration * 0.343 / 2.0;
    }

    return 300; 
}
