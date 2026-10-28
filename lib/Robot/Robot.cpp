#include <Robot.h>

Robot::Robot(MotorControl& motorsRef, Scanner& scannerRef, WebInterface& webRef, AutonomousControl& autoRef):

    motors(motorsRef),
    scanner(scannerRef),
    webInt(webRef),
    autonomousControl(autoRef)

{
}

CommandType Robot::transformPowerButtonIntoCommand() {
    if (this->mode != RobotMode::SHUTDOWN) {
        return CommandType::SHUTDOWN;
    } else {
        return CommandType::START;
    }
}

void Robot::clearCommandsQueue() {
    while (!this->commandQueue.empty()) {
        this->commandQueue.pop();
    }
}

void Robot::reset() {
    this->motors.setToNeutral();

    this->scanner.resetScanning();
    this->scanner.resetSensorFilter();

    this->autonomousControl.resetState();

    this->clearCommandsQueue();
    this->driveCommandReady = false;
}

void Robot::startWifiConnection() {
    WiFi.setHostname("esp32-masina-ultrasunete");
    WiFi.begin(this->ssid, this->pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

bool Robot::isCommandValid(const RobotCommands& command) {
    //Comenzi acceptate in modul SCANNING
    if (this->mode == RobotMode::AUTONOMOUS && this->autonomousControl.getState() == AutonomousState::SCANNING) {
        if (
            command.type != CommandType::SHUTDOWN &&
            command.type != CommandType::DISCONNECTED &&
            command.type != CommandType::CHANGE_MODE
        ) {
            return false;
        }
    }
    
    //Scope compatibil cu modul robotului
    if (this->mode == RobotMode::MANUAL && command.scope == CommandScope::AUTONOMOUS) {
        return false;
    }

    if (this->mode == RobotMode::AUTONOMOUS && command.scope == CommandScope::MANUAL) {
        return false;
    }

    //Cand robotul e oprit, se accepta doar comanda de pornire;
    if (this->mode == RobotMode::SHUTDOWN && command.type != CommandType::START) {
        return false;
    }

    //Comanda NONE nu se executa
    if (command.type == CommandType::NONE) {
        return false;
    }

    //Se verifica relatia source <-> type
    if (
        command.type == CommandType::MANUAL_DRIVE &&
        command.source != CommandSource::WEB_INTERFACE
    ) {
        return false;
    }

    if (
        (
            command.type == CommandType::FORWARD_AUTO ||
            command.type == CommandType::BACKWARD_AUTO ||
            command.type == CommandType::STEERING_AUTO_LEFT ||
            command.type == CommandType::STEERING_AUTO_RIGHT
        )
        &&
        command.source != CommandSource::AUTONOMOUS
    ) {
        return false;
    }

    //Verificarea valorilor pentru MANUAL_DRIVE
    if (command.type == CommandType::MANUAL_DRIVE) {

        if (
            command.throttleVal < 1000 ||
            command.throttleVal > 2000
        ) {
            return false;
        }

        if (
            command.steeringVal < 1000 ||
            command.steeringVal > 2000
        ) {
            return false;
        }
    }

    return true;
}

void Robot::startRobot() {
    this->motors.setToNeutral();
    this->scanner.startScanner();

    this->startWifiConnection();
    this->webInt.initializeWebConnection();
}

void Robot::update() {
    if (this->mode != RobotMode::AUTONOMOUS || this->autonomousControl.getState() != AutonomousState::SCANNING) {
        if (this->scanner.update()) {
            this->webInt.sendDistance(this->scanner.getFilteredDistance());
        }
        
    } else {
        this->webInt.sendDistance(-1);
    }
    this->webInt.update();

    while (this->webInt.hasCommands()) {
        RobotCommands command = this->webInt.popPendingCommand();

        if (command.type == CommandType::POWERBUTTON) {
            command.type = this->transformPowerButtonIntoCommand();
        }

        if (this->isCommandValid(command) && (this->mode != RobotMode::SHUTDOWN || command.type == CommandType::START)) {
            this->commandQueue.push(command);
        }

    }

    if (this->webInt.isDriveCommandReady()) {
        RobotCommands command = this->webInt.getLatestDriveCommand();

        if (this->isCommandValid(command) && this->mode != RobotMode::SHUTDOWN) {
            this->latestDriveCommand = command;
            this->driveCommandReady = true;
        }
    }

    if (this->mode == RobotMode::AUTONOMOUS) {
        if (this->autonomousControl.getState() == AutonomousState::SCANNING) {
            if (this->scanner.scanning()) {
                float left = this->scanner.getLeftDistance();
                float right = this->scanner.getRightDistance();

                RobotCommands command = this->autonomousControl.obstacleAvoidance(-1, left, right);
                if (this->isCommandValid(command)) {
                    this->commandQueue.push(command);
                    latestAutonomousCommand = command;
                }

                this->scanner.resetSensorFilter();

            } 
        } else {
            RobotCommands command = this->autonomousControl.obstacleAvoidance(this->scanner.getFilteredDistance());
            if (this->isCommandValid(command) && command.type != latestAutonomousCommand.type) {
                this->commandQueue.push(command);
                latestAutonomousCommand = command;
            }
        }
    }

    this->processComands();

    if (this->mode == RobotMode::AUTONOMOUS) {
        AutonomousState currentState = this->autonomousControl.getState();
        if (lastState != currentState) {
            this->webInt.sendAutonomousState(currentState);
            lastState = currentState;
        }
    }
}

void Robot::processComands() {
    constexpr int MAX_COMMANDS_PER_UPDATE = 5;
    int processed = 0;

    while (processed < MAX_COMMANDS_PER_UPDATE && !this->commandQueue.empty()) {
        RobotCommands command = this->commandQueue.front();

        this->commandQueue.pop();

        if (this->isCommandValid(command)) {
            this->executeCommand(command);
        }

        processed++;
    }

    if (this->driveCommandReady) {
        this->driveCommandReady = false;

        if (this->isCommandValid(this->latestDriveCommand)) {
            this->executeCommand(this->latestDriveCommand);
        }
    }
}

void Robot::executeCommand(const RobotCommands& command) {
    switch (command.type) {

        case CommandType::CONNECTED:
            this->reset();
            this->mode = RobotMode::MANUAL;
            this->webInt.sendRobotMode(this->mode);
            break;

        case CommandType::CONNECTION_REJECTED:
            
            break;

        case CommandType::DISCONNECTED:
            this->reset();
            break;

        case CommandType::NONE:
            
            break;

        case CommandType::FORWARD_AUTO:
            this->motors.steeringNeutral();
            this->motors.forward();
            break;

        case CommandType::BACKWARD_AUTO:
            this->motors.backward();
            break;

        case CommandType::BRAKE_AUTO:
            this->motors.setToNeutral();
            break;

        case CommandType::RIGHT_LEFT_DISTANCES:
            this->motors.setToNeutral();
            this->scanner.scanning();

            break;

        case CommandType::STEERING_AUTO_LEFT:
            this->motors.steeringLeft();
            this->motors.forward();
            break;

        case CommandType::STEERING_AUTO_RIGHT:
            this->motors.steeringRight();
            this->motors.forward();
            break;

        case CommandType::MANUAL_DRIVE:
            this->motors.setThrottleVal(command.throttleVal);
            this->motors.setSteeringVal(command.steeringVal);
            break;

        case CommandType::STEERING_TRIMMER:
            this->motors.updateSteeringOffset(command.steeringTrimmPositive);
            this->webInt.sendSteeringNeutralValue(this->motors.getNeutralSteeringValue());
            break;

        case CommandType::CHANGE_MODE:
            if (this->mode == RobotMode::SHUTDOWN) {
                break;
            }

            this->reset();

            if (this->mode == RobotMode::MANUAL) {
                this->mode = RobotMode::AUTONOMOUS;
            } else {
                this->mode = RobotMode::MANUAL;
            }

            this->webInt.sendRobotMode(this->mode);

            break;

        case CommandType::START:
            this->reset();
            this->mode = RobotMode::MANUAL;
            this->webInt.sendRobotMode(this->mode);
            this->webInt.sendStateLightMode(true);
            break;

        case CommandType::SHUTDOWN:
            this->reset();
            this->mode = RobotMode::SHUTDOWN;
            this->webInt.sendStateLightMode(false);
            break;

        default:
            
            break;
    }
}