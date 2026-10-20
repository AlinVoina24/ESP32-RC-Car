#pragma once

enum class ErrorTypes {
    UNKNOWN_WEB_INTERFACE_MODE,

};

enum class AutonomousState {
  MOVING_FORWARD,
  MOVING_BACKWARD,
  SCANNING,
  STEERING,
  WAIT_FOR_DISTANCE,
  NONE
};

enum class RobotMode {
    MANUAL,
    AUTONOMOUS,
    SHUTDOWN
};

enum class CommandScope {
    MANUAL,
    AUTONOMOUS,
    BOTH
};

enum class CommandSource {
    WEB_INTERFACE,
    AUTONOMOUS,
};

enum class CommandType {
    CONNECTED,
    CONNECTION_REJECTED,
    DISCONNECTED,
    NONE,
    FORWARD_AUTO,
    BACKWARD_AUTO,
    BRAKE_AUTO,
    STEERING_AUTO_LEFT,
    STEERING_AUTO_RIGHT, 
    MANUAL_DRIVE,
    STEERING_TRIMMER,
    RIGHT_LEFT_DISTANCES,
    CHANGE_MODE,
    POWERBUTTON,
    START,
    SHUTDOWN,
    ERROR_LOG
};

struct RobotCommands {
    CommandSource source;
    CommandScope scope;
    CommandType type;

    int throttleVal = 1500;
    int steeringVal = 1500;

    bool steeringTrimmPositive = false;
};