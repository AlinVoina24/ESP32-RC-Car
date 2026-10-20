#include <RobotCommands.h>
#include <string>

std::string autonomousStateToString(AutonomousState state) {
    if (state == AutonomousState::WAIT_FOR_DISTANCE) {
        return "WAIT_FOR_DISTANCE";
    }
    
    if (state == AutonomousState::MOVING_FORWARD) {
        return "MOVING_FORWARD";
    }

    if (state == AutonomousState::MOVING_BACKWARD) {
        return "MOVING_BACKWARD";
    }

    if (state == AutonomousState::SCANNING) {
        return "SCANNING";
    }

    if (state == AutonomousState::STEERING) {
        return "STEERING";
    }

    return "UNKNOWN_STATE";
}

std::string robotModeToString(RobotMode mode) {
    if (mode == RobotMode::MANUAL) {
        return "MANUAL";
    }
    if (mode == RobotMode::AUTONOMOUS) {
        return "AUTONOMOUS";
    }

    return "UNKNOWN";
}
