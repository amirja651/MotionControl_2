#ifndef TMC5160_MANAGER_H
#define TMC5160_MANAGER_H

#include "DriverPins.h"
#include "SPIPins.h"
#include <Arduino.h>
#include <SPI.h>
#include <TMCStepper.h>

// Derived class to access protected methods
class TMC5160StepperExtended : public TMC5160Stepper
{
public:
    TMC5160StepperExtended(uint16_t pinCS, float RS) : TMC5160Stepper(pinCS, RS) {}
    using TMC5160Stepper::read;
    using TMC5160Stepper::write;
};

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

    // Motion Control
    void stopMotor(uint8_t driverIndex);
    void emergencyStop();

private:
    static constexpr uint8_t  NUM_DRIVERS     = 4;
    static constexpr uint32_t SPI_CLOCK       = 1000000;  // 1MHz SPI clock
    static constexpr float    R_SENSE         = 0.11f;    // Sense resistor value in ohms (updated value)
    static constexpr uint16_t DEFAULT_CURRENT = 1000;     // Default current in mA

    TMC5160StepperExtended* drivers[NUM_DRIVERS];
    bool                    driverEnabled[NUM_DRIVERS];
    int32_t                 currentPositions[NUM_DRIVERS];

    // Private helper methods
    void     configureDriver(uint8_t driverIndex);
    void     setSGTHRS(uint8_t driverIndex, uint32_t threshold);
    uint32_t getSG_RESULT(uint8_t driverIndex);
};

#endif  // TMC5160_MANAGER_H