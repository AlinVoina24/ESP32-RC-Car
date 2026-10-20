#pragma once

#include <MotorControl.h>
#include <UltraSounds.h>
#include <RobotCommands.h>

class AutonomousControl {
    private:
        AutonomousState state = AutonomousState::WAIT_FOR_DISTANCE;

        unsigned long steeringStartTime;

    public:
        AutonomousControl() = default;
        
        RobotCommands obstacleAvoidance(float forwardDistance = -1, float leftDistance = -1, float rightDistance = -1);
        AutonomousState getState();

        void resetState();
};