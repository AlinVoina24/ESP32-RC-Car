#pragma once

#include <ESP32Servo.h>

class MotorControl {
    private:
        Servo throttle;
        Servo steering;

        int throttleOffset = 0;
        int steeringOffset = 0;

    public:
        MotorControl(int, int);

        int getNeutralSteeringValue();

        void setToNeutral();

        void forward();
        void backward();
        void brake();

        void steeringRight();
        void steeringLeft();
        void steeringNeutral();

        void setThrottleVal(int);
        void setSteeringVal(int);

        void updateThrottleOffset(bool);
        void updateSteeringOffset(bool);
};