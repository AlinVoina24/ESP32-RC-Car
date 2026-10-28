#pragma once

#include <KalmanFilter.h>
#include <DistanceSensor.h>

#include <Arduino.h>

class UltrasonicSensor : public DistanceSensor {
    private:
        int trigPin;
        int echoPin;
    
    public:
        UltrasonicSensor(int, int);

        bool begin() override;
        float getDistance() override;
        
};