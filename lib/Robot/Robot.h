#pragma once

#include <queue>

#include <MotorControl.h>
#include <UltraSounds.h>
#include <WebInterface.h>
#include <AutonomousControl.h>
#include <RobotCommands.h>
#include <testing_env.h>

#include <WiFi.h>

class Robot {
    private:
        MotorControl& motors;
        UltraSounds& sensor;
        WebInterface& webInt;
        AutonomousControl&  autonomousControl;

        RobotMode mode = RobotMode::SHUTDOWN;
        AutonomousState lastState = AutonomousState::NONE;

        std::queue<RobotCommands> commandQueue;
        RobotCommands latestDriveCommand{};
        bool driveCommandReady= false;
        RobotCommands latestAutonomousCommand{};
        
        const char* ssid = ssid_TEST;
        const char* pass = pass_TEST;

        CommandType transformPowerButtonIntoCommand();

        void clearCommandsQueue();
        void reset();

        void startWifiConnection();
        bool isCommandValid(const RobotCommands&);

        void processComands();
        void executeCommand(const RobotCommands&);
    
    public:
        Robot(MotorControl&, UltraSounds&, WebInterface&, AutonomousControl&);

        void startRobot();
        void update();
};