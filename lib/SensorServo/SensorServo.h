#pragma once

#include <string>
#include <map>

#include <RobotCommands.h>

#include <ESP32Servo.h>

class SensorServo {
    private:
        Servo sensorServo;
        int servoOffset = 0;
        std::map<SensorDirections, int> dirs = {{SensorDirections::MID, 90}, {SensorDirections::RIGHT, 40}, {SensorDirections::LEFT, 140}};

        std::string dirToString(SensorDirections);
    public:
        SensorServo(int);

        void moveSensor(SensorDirections);
        void updateServoOffset(bool);
        int getServoOffset();
};