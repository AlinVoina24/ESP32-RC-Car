#pragma once
#include <map>

#include <KalmanFilter.h>

#include <ESP32Servo.h>

#include <string>

enum class SensorDirections {
    MID,
    LEFT,
    RIGHT
};

enum class ScanningState {
    WAITING,
    MOVING_LEFT,
    MOVING_RIGHT,
    MOVING_MID,
    SCANNING_LEFT,
    SCANNING_RIGHT,
    READY
};

class UltraSounds {
    private:
        Servo sensorServo;

        int trigPin;
        int echoPin;
        int servoOffset = -10;

        std::map<SensorDirections, int> dirs = {{SensorDirections::MID, 90}, {SensorDirections::RIGHT, 40}, {SensorDirections::LEFT, 140}};

        int sampleTimePeriod = 35;
        unsigned long lastSampleTime = 0;
        float MAX_JUMP = 50;
        int suspiciousCount = 0;

        float distance = -1;

        ScanningState scanningState = ScanningState::WAITING;
        unsigned long movingStartTime;
        float values[3];
        int sampleIndex = 0;
        unsigned long lastSampleTimeRL;
        float rightDistance = -1;
        float leftDistance = -1;

        KalmanFilter kf = KalmanFilter(0.01f, 4.0f);

        float getDistance();
        float getMedian();
        std::string dirToString(SensorDirections);

    public:
        UltraSounds(int, int, int);

        float getMedianDistance();
        bool update();
        float getFilteredDistance();
        void resetKalmanFilter();

        void moveSensor(SensorDirections);
        void updateServoOffset(bool);

        bool scanning();
        void resetScanning();
        float getRightDistance();
        float getLeftDistance();
};