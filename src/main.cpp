#include <Arduino.h>

#include <MotorControl.h>
#include <UltraSounds.h>
#include <SensorServo.h>
#include <KalmanFilter.h>
#include <Scanner.h>
#include <WebInterface.h>
#include <AutonomousControl.h>
#include <Robot.h>

MotorControl motors(27, 26);
UltrasonicSensor sensor(25, 34);
SensorServo servo(33);
KalmanFilter kf = KalmanFilter(0.01f, 4.0f);
Scanner scanner(sensor, kf, servo);
WebInterface webInt;
AutonomousControl autonomousControl;
Robot robot(motors, scanner, webInt, autonomousControl);

void setup() {
  robot.startRobot();
}

void loop() {
    robot.update();
}