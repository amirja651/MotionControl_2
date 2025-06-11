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
        drivers[i] = new TMC5160StepperExtended(DriverPins::CS[i], R_SENSE);

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

    status.connected  = true;
    status.version    = drivers[driverIndex]->version();
    status.status     = drivers[driverIndex]->DRV_STATUS();
    status.stallGuard = drivers[driverIndex]->TCOOLTHRS();

    // Safely get current value
    try
    {
        status.current = drivers[driverIndex]->rms_current();
    }
    catch (...)
    {
        status.current = 0;
    }

    status.temperature = drivers[driverIndex]->TSTEP();

    return status;
}

void TMC5160Manager::configureDriver(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
    {
        return;
    }

    TMC5160StepperExtended* driver = drivers[driverIndex];

    // Basic configuration
    driver->begin();
    driver->toff(5);                       // Enable driver
    driver->rms_current(DEFAULT_CURRENT);  // Set current to 1A
    driver->microsteps(16);                // Set microsteps to 16

    // Configure spreadCycle
    driver->en_pwm_mode(false);   // Disable stealthChop
    driver->pwm_autoscale(true);  // Enable automatic current scaling

    // StallGuard configuration
    driver->TCOOLTHRS(0xFFFFF);   // 20bit max
    setSGTHRS(driverIndex, 100);  // Stall threshold

    // Enable the driver
    enableDriver(driverIndex, true);

    // Print configuration
    Serial.printf("Driver %d configured:\n", driverIndex);
    Serial.printf("  Current: %d mA\n", DEFAULT_CURRENT);
    Serial.printf("  Microsteps: 16\n");
    Serial.printf("  Mode: STEP/DIR with spreadCycle\n");

    // Read and print important registers
    uint32_t drv_status = driver->DRV_STATUS();
    uint32_t gconf      = driver->read(0x00);
    Serial.printf("  DRV_STATUS: 0x%08X\n", drv_status);
    Serial.printf("  GCONF: 0x%08X\n", gconf);
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

void TMC5160Manager::stopMotor(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
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

void TMC5160Manager::setSGTHRS(uint8_t driverIndex, uint32_t threshold)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return;
    drivers[driverIndex]->write(0x40, threshold);  // Direct register access
}

uint32_t TMC5160Manager::getSG_RESULT(uint8_t driverIndex)
{
    if (driverIndex >= NUM_DRIVERS || !drivers[driverIndex])
        return 0;
    return drivers[driverIndex]->sg_result();  // TMC5160 uses lowercase method names
}
