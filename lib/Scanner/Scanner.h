#pragma once

#include <UltrasonicSensor.h>
#include <SensorServo.h>
#include <RobotCommands.h>

class Scanner {
    private:
        DistanceSensor& sensor;
        DistanceFilter& filter;
        SensorServo& servo;


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

        float getMedian();

    public:
        Scanner(DistanceSensor&, DistanceFilter&, SensorServo&);

        void startScanner();
        bool update();
        float getFilteredDistance();
        void resetSensorFilter();

        bool scanning();
        void resetScanning();
        float getRightDistance();
        float getLeftDistance();
};