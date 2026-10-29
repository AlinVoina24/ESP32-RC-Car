#include <AutonomousControl.h>
#include <RobotCommands.h>

RobotCommands AutonomousControl::obstacleAvoidance(float forwardDistance, float leftDistance, float rightDistance) {
    RobotCommands command{};
    command.source = CommandSource::AUTONOMOUS;
    command.scope = CommandScope::AUTONOMOUS;
    command.type = CommandType::NONE;
    
    switch(this->state) {
        case AutonomousState::WAIT_FOR_DISTANCE:
            if (forwardDistance >= 0) {
                if (forwardDistance > 70) {
                    command.type = CommandType::FORWARD_AUTO;
                    this->state = AutonomousState::MOVING_FORWARD;
                } else if (forwardDistance <= 70) {
                    command.type = CommandType::BACKWARD_AUTO;
                    this->state = AutonomousState::MOVING_BACKWARD;
                }
            } else {
                command.type = CommandType::NONE;
            }
            break;

        case AutonomousState::MOVING_FORWARD:
            if (forwardDistance < 0) {
                command.type = CommandType::BRAKE_AUTO;
                this->state = AutonomousState::WAIT_FOR_DISTANCE;
                break;
            }

            if (forwardDistance <= 70) {
                command.type = CommandType::BRAKE_AUTO;
                this->state = AutonomousState::MOVING_BACKWARD;
            } else {
                command.type = CommandType::FORWARD_AUTO;
            }
            break;
        
        case AutonomousState::MOVING_BACKWARD:
            if (forwardDistance < 0) {
                command.type = CommandType::BRAKE_AUTO;
                this->state = AutonomousState::WAIT_FOR_DISTANCE;
                break;
            }

            if (forwardDistance < 100) {
                command.type = CommandType::BACKWARD_AUTO;
            } else {
                command.type = CommandType::BRAKE_AUTO;
                this->state = AutonomousState::STOPPING;
                this->stopingStartTime = millis();
            }
            break;

        case AutonomousState::STOPPING: {
            unsigned long now = millis();

            if (now - this->stopingStartTime >= 1000) {
                command.type = CommandType::RIGHT_LEFT_DISTANCES;
                this->state = AutonomousState::SCANNING;
            } else {
                command.type = CommandType::NONE;
            }
            break;
        }

        case AutonomousState::SCANNING:
            if (leftDistance < 0 || rightDistance < 0) {
                command.type = CommandType::RIGHT_LEFT_DISTANCES;
                break;
            }

            if (leftDistance < 50 && rightDistance < 50) {
                command.type = CommandType::SHUTDOWN;
                this->state = AutonomousState::WAIT_FOR_DISTANCE;
                break;
            }
            
            if (rightDistance > leftDistance) {
                command.type = CommandType::STEERING_AUTO_RIGHT;
            } else {
                command.type = CommandType::STEERING_AUTO_LEFT;
            }

            this->steeringStartTime = millis();
            this->state = AutonomousState::STEERING;
            break;
        
        case AutonomousState::STEERING: {
            unsigned long now = millis();
            if (now - this->steeringStartTime >= 1000) {
                command.type = CommandType::FORWARD_AUTO;
                this->state = AutonomousState::MOVING_FORWARD;
            }
            
            break;
        }

        default:
        //nu stiu ce sa scriu aici
        command.type = CommandType::SHUTDOWN;
        this->state = AutonomousState::WAIT_FOR_DISTANCE;
        break;
    }

    return command;
}

AutonomousState AutonomousControl::getState() {
    return this->state;
}

void AutonomousControl::resetState() {
    this->state = AutonomousState::WAIT_FOR_DISTANCE;
}