#include "SystemDiagnostics.h"
#include "TMC5160Manager.h"
#include <Arduino.h>
#include <SPI.h>
#include <SimpleCLI.h>
#include <TMCStepper.h>
#include <esp_task_wdt.h>

TMC5160Manager driverManager;

void setup()
{
    SPI.begin();
    Serial.begin(115200);
    delay(1000);
    while (!Serial)
    {
        delay(10);
    }

    // Initialize and print system diagnostics
    SystemDiagnostics::initialize();
    SystemDiagnostics::printSystemInfo();
    SystemDiagnostics::printSystemStatus();

    // Initialize TMC5160 drivers
    if (!driverManager.begin())
    {
        Serial.println("Failed to initialize TMC5160 drivers!");
        while (1)
            delay(1000);
    }

    // Test connection for each driver
    for (uint8_t i = 0; i < 4; i++)
    {
        if (driverManager.testConnection(i))
        {
            Serial.printf("Driver %d connected successfully\n", i);

            // Get and print driver status
            auto status = driverManager.getDriverStatus(i);
            Serial.printf("Driver %d Status:\n", i);
            Serial.printf("  Version: 0x%08X\n", status.version);
            Serial.printf("  Current: %d mA\n", status.current);
            Serial.printf("  Temperature: %d\n", status.temperature);

            // Enable the driver
            driverManager.enableDriver(i, true);

            // Configure motor parameters
            driverManager.setCurrent(i, 1000);   // 1A current
            driverManager.setMicrosteps(i, 16);  // 16 microsteps
        }
        else
        {
            Serial.printf("Driver %d connection failed!\n", i);
        }
    }
}

void loop()
{
    esp_task_wdt_reset();

    // Print motor status every 2 seconds
    static uint32_t lastStatusTime = 0;
    if (millis() - lastStatusTime >= 2000)
    {
        lastStatusTime = millis();

        if (driverManager.testConnection(0))
        {
            auto status = driverManager.getDriverStatus(0);
            Serial.printf("Motor Status:\n");
            Serial.printf("  Current: %d mA\n", status.current);
            Serial.printf("  Temperature: %d\n", status.temperature);
        }
    }

    vTaskDelay(1);
}
