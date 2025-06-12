#include "MotorSpeedController.h"

// Define the static constexpr array
constexpr uint8_t MotorSpeedController::LEDC_CHANNELS[4];

MotorSpeedController::MotorSpeedController(TMC5160Manager& driverManager, uint8_t motorIndex)
    : driverManager(driverManager), motorIndex(motorIndex), currentSpeed(0), isRunning(false)
{
}

void MotorSpeedController::begin()
{
    if (motorIndex >= NUM_DRIVERS)
        return;

    // Configure LEDC
    ledcSetup(LEDC_CHANNELS[motorIndex], LEDC_BASE_FREQ, LEDC_TIMER_BITS);
    // Attach LEDC channels to step pins
    ledcAttachPin(DriverPins::STEP[motorIndex], LEDC_CHANNELS[motorIndex]);

    // Initialize motor
    driverManager.enableDriver(motorIndex, false);
    stop();
}

void MotorSpeedController::setSpeed(int16_t speed)
{
    if (motorIndex >= NUM_DRIVERS)
        return;

    // Constrain speed to -100 to 100 range
    speed = constrain(speed, -100, 100);

    if (speed == 0)
    {
        stop();
        return;
    }

    // Set direction based on speed sign
    setDirection(speed > 0);

    // Convert speed to PWM value (0-255)
    uint32_t pwmValue = map(abs(speed), 0, 100, 0, 255);
    ledcWrite(LEDC_CHANNELS[motorIndex], pwmValue);

    currentSpeed = speed;
    isRunning    = true;
}

void MotorSpeedController::stop()
{
    if (motorIndex >= NUM_DRIVERS)
        return;

    ledcWrite(LEDC_CHANNELS[motorIndex], 0);
    currentSpeed = 0;
    isRunning    = false;
}

void MotorSpeedController::emergencyStop()
{
    stop();
    driverManager.emergencyStop();
}

bool MotorSpeedController::isMoving() const
{
    return isRunning;
}

int16_t MotorSpeedController::getCurrentSpeed() const
{
    return currentSpeed;
}

void MotorSpeedController::setDirection(bool forward)
{
    driverManager.setDirection(motorIndex, forward);
}

void MotorSpeedController::updatePWM()
{
    if (motorIndex >= NUM_DRIVERS)
        return;

    if (isRunning)
    {
        uint32_t pwmValue = map(abs(currentSpeed), 0, 100, 0, 255);
        ledcWrite(LEDC_CHANNELS[motorIndex], pwmValue);
    }
}