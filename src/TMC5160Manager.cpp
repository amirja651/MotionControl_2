#include "TMC5160Manager.h"

TMC5160Manager::TMC5160Manager()
{
    for (uint8_t i = 0; i < NUM_DRIVERS; i++)
    {
        drivers[i]          = nullptr;
        driverEnabled[i]    = false;
        currentPositions[i] = 0;
    }
}

bool TMC5160Manager::begin()
{
    // Initialize SPI
    SPI.begin(SPIPins::SCK, SPIPins::MISO, SPIPins::MOSI);
    SPI.setFrequency(SPI_CLOCK);
    SPI.setDataMode(SPI_MODE3);

    // Initialize each driver
    for (uint8_t i = 0; i < NUM_DRIVERS; i++)
    {
        // Create new driver instance
        drivers[i] = new TMC5160Stepper(DriverPins::CS[i], R_SENSE);

        // Configure pins
        pinMode(DriverPins::DIR[i], OUTPUT);
        pinMode(DriverPins::STEP[i], OUTPUT);
        pinMode(DriverPins::EN[i], OUTPUT);

        // Default state
        digitalWrite(DriverPins::EN[i], HIGH);  // Disable driver initially
        digitalWrite(DriverPins::DIR[i], LOW);
        digitalWrite(DriverPins::STEP[i], LOW);

        // Configure driver
        configureDriver(i);
    }

    return true;
}

bool TMC5160Manager::testConnection(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
    {
        return false;
    }

    // Try to read the version register
    uint32_t version = drivers[driverIndex]->version();
    return (version != 0 && version != 0xFFFFFFFF);
}

TMC5160Manager::DriverStatus TMC5160Manager::getDriverStatus(uint8_t driverIndex)
{
    DriverStatus status = {false, 0, 0, 0, 0, 0};

    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
    {
        return status;
    }

    status.connected   = true;
    status.version     = drivers[driverIndex]->version();
    status.status      = drivers[driverIndex]->DRV_STATUS();
    status.stallGuard  = drivers[driverIndex]->TCOOLTHRS();
    status.current     = drivers[driverIndex]->rms_current();
    status.temperature = drivers[driverIndex]->TSTEP();

    return status;
}

void TMC5160Manager::configureDriver(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
    {
        return;
    }

    TMC5160Stepper* driver = drivers[driverIndex];

    // Basic configuration
    driver->begin();
    driver->toff(5);              // Enable driver
    driver->rms_current(1000);    // Set current to 1A
    driver->microsteps(16);       // Set microsteps to 16
    driver->en_pwm_mode(true);    // Enable stealthChop
    driver->pwm_autoscale(true);  // Enable automatic current scaling

    // Motion profile settings
    driver->VSTART(0);     // Start velocity
    driver->A1(1000);      // First acceleration
    driver->V1(100000);    // First velocity
    driver->AMAX(2000);    // Maximum acceleration
    driver->VMAX(200000);  // Maximum velocity
    driver->DMAX(2000);    // Maximum deceleration
    driver->D1(1000);      // First deceleration
    driver->VSTOP(10);     // Stop velocity
    driver->TZEROWAIT(0);  // Zero crossing wait time

    // StallGuard configuration
    driver->TCOOLTHRS(0xFFFFF);  // 20bit max
    driver->SGTHRS(100);         // Stall threshold
}

void TMC5160Manager::setDirection(uint8_t driverIndex, bool direction)
{
    if (driverIndex >= NUM_DRIVERS)
        return;
    digitalWrite(DriverPins::DIR[driverIndex], direction);
}

void TMC5160Manager::enableDriver(uint8_t driverIndex, bool enable)
{
    if (driverIndex >= NUM_DRIVERS)
        return;
    digitalWrite(DriverPins::EN[driverIndex], !enable);
    driverEnabled[driverIndex] = enable;
}

void TMC5160Manager::setCurrent(uint8_t driverIndex, uint16_t current)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->rms_current(current);
}

void TMC5160Manager::setMicrosteps(uint8_t driverIndex, uint16_t microsteps)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->microsteps(microsteps);
}

void TMC5160Manager::setMaxSpeed(uint8_t driverIndex, uint32_t speed)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->VMAX(speed);
}

void TMC5160Manager::setAcceleration(uint8_t driverIndex, uint32_t acceleration)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->AMAX(acceleration);
}

void TMC5160Manager::moveToPosition(uint8_t driverIndex, int32_t position)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->XTARGET(position);
    currentPositions[driverIndex] = position;
}

void TMC5160Manager::moveRelative(uint8_t driverIndex, int32_t distance)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    int32_t newPosition = currentPositions[driverIndex] + distance;
    moveToPosition(driverIndex, newPosition);
}

void TMC5160Manager::stopMotor(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->XTARGET(drivers[driverIndex]->XACTUAL());
}

void TMC5160Manager::emergencyStop()
{
    for (uint8_t i = 0; i < NUM_DRIVERS; i++)
    {
        if (drivers[i])
        {
            stopMotor(i);
            enableDriver(i, false);
        }
    }
}

bool TMC5160Manager::isMoving(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return false;
    return (drivers[driverIndex]->VACTUAL() != 0);
}

int32_t TMC5160Manager::getCurrentPosition(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return 0;
    return drivers[driverIndex]->XACTUAL();
}

uint32_t TMC5160Manager::getCurrentSpeed(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return 0;
    return drivers[driverIndex]->VACTUAL();
}