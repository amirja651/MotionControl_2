#ifndef MOTOR_SPEED_CONTROLLER_H
#define MOTOR_SPEED_CONTROLLER_H

#include "DriverPins.h"
#include "TMC5160Manager.h"
#include <Arduino.h>

class MotorSpeedController
{
public:
    // Constructor
    MotorSpeedController(TMC5160Manager& driverManager, uint8_t motorIndex);

    // Initialize the controller
    void begin();

    // Speed control methods
    void setSpeed(int16_t speed);  // Speed range: -100 to 100
    void stop();
    void emergencyStop();

    // Status methods
    bool    isMoving() const;
    int16_t getCurrentSpeed() const;

private:
    static constexpr uint8_t  LEDC_CHANNELS[NUM_DRIVERS] = {0, 1, 2, 3};
    static constexpr uint8_t  LEDC_TIMER_BITS            = 10;
    static constexpr uint32_t LEDC_BASE_FREQ             = 5000;  // 5kHz base frequency

    TMC5160Manager& driverManager;
    uint8_t         motorIndex;
    int16_t         currentSpeed;
    bool            isRunning;

    // Helper methods
    void updatePWM();
    void setDirection(bool forward);
};

#endif  // MOTOR_SPEED_CONTROLLER_H