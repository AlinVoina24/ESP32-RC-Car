#include <Arduino.h>
#include <MotorControl.h>
#include <UltraSounds.h>
#include <WebInterface.h>
#include <AutonomousControl.h>
#include <Robot.h>

MotorControl motors(27, 26);
UltraSounds sensor(25, 34, 33);
WebInterface webInt;
AutonomousControl autonomousControl;
Robot robot(motors, sensor, webInt, autonomousControl);

void setup() {
  robot.startRobot();
}

void loop() {
    robot.update();
}