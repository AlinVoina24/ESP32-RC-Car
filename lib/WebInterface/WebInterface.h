#pragma once

#include <queue>

#include <RobotCommands.h>

#include <WebServer.h>
#include <WebSocketsServer.h>

class WebInterface {
    private:
        WebServer server;
        WebSocketsServer webSocket;
        
        int activeUser = -1;
        
        void webSocketsEvents(uint8_t, WStype_t, uint8_t*, size_t);

        std::queue<RobotCommands> pendingCommands;
        RobotCommands latestDriveCommand{};
        bool driveCommandReady = false;

    public:
        WebInterface();

        void initializeWebConnection();
        void update();

        const bool hasCommands();
        RobotCommands popPendingCommand();

        const bool isDriveCommandReady();
        RobotCommands getLatestDriveCommand();

        void sendErrorToLog();
        void sendAutonomousState(AutonomousState);
        void sendRobotMode(RobotMode);
        void sendStateLightMode(bool);
        void sendSteeringNeutralValue(int);
        void sendDistance(float);
        void sendThrottleSteering(int, int);
        void sendMessage(std::string, std::string);
};