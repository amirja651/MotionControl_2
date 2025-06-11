#ifndef TMC5160_MANAGER_H
#define TMC5160_MANAGER_H

#include "DriverPins.h"
#include "SPIPins.h"
#include <Arduino.h>
#include <SPI.h>
#include <TMCStepper.h>


class TMC5160Manager
{
public:
    // Constructor
    TMC5160Manager();

    // Initialization
    bool begin();
    bool testConnection(uint8_t driverIndex);

    // Driver Status
    struct DriverStatus
    {
        bool     connected;
        uint32_t version;
        uint32_t status;
        uint32_t stallGuard;
        uint32_t current;
        uint32_t temperature;
    };

    DriverStatus getDriverStatus(uint8_t driverIndex);

    // Motor Control
    void setDirection(uint8_t driverIndex, bool direction);
    void enableDriver(uint8_t driverIndex, bool enable);
    void setCurrent(uint8_t driverIndex, uint16_t current);
    void setMicrosteps(uint8_t driverIndex, uint16_t microsteps);
    void setMaxSpeed(uint8_t driverIndex, uint32_t speed);
    void setAcceleration(uint8_t driverIndex, uint32_t acceleration);

    // Motion Control
    void moveToPosition(uint8_t driverIndex, int32_t position);
    void moveRelative(uint8_t driverIndex, int32_t distance);
    void stopMotor(uint8_t driverIndex);
    void emergencyStop();

    // Status Monitoring
    bool     isMoving(uint8_t driverIndex);
    int32_t  getCurrentPosition(uint8_t driverIndex);
    uint32_t getCurrentSpeed(uint8_t driverIndex);

private:
    static constexpr uint8_t  NUM_DRIVERS = 4;
    static constexpr uint32_t SPI_CLOCK   = 1000000;  // 1MHz SPI clock
    static constexpr uint32_t R_SENSE     = 0.075f;   // Sense resistor value in ohms

    TMC5160Stepper* drivers[NUM_DRIVERS];
    bool            driverEnabled[NUM_DRIVERS];
    int32_t         currentPositions[NUM_DRIVERS];

    // Private helper methods
    void configureDriver(uint8_t driverIndex);
    bool checkDriverVersion(uint8_t driverIndex);
    void updateDriverStatus(uint8_t driverIndex);
};

#endif  // TMC5160_MANAGER_H