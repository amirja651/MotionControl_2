#include "MotorSpeedController.h"
#include "SystemDiagnostics.h"
#include "TMC5160Manager.h"
#include <Arduino.h>
#include <SPI.h>
#include <SimpleCLI.h>
#include <TMCStepper.h>
#include <esp_task_wdt.h>

TMC5160Manager driverManager;
// Create controllers for all motors
MotorSpeedController motor0(driverManager, 0);
MotorSpeedController motor1(driverManager, 1);
MotorSpeedController motor2(driverManager, 2);
MotorSpeedController motor3(driverManager, 3);

// Task handles
TaskHandle_t encoderUpdateTaskHandle = NULL;
TaskHandle_t motorUpdateTaskHandle   = NULL;
TaskHandle_t serialReadTaskHandle    = NULL;
TaskHandle_t serialPrintTaskHandle   = NULL;

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

            // Initialize all motor controllers
            switch (i)
            {
                case 0:
                    motor0.begin();
                    break;
                case 1:
                    motor1.begin();
                    break;
                case 2:
                    motor2.begin();
                    break;
                case 3:
                    motor3.begin();
                    break;
            }
        }
        else
        {
            Serial.printf("Driver %d connection failed!\n", i);
        }
    }

    xTaskCreatePinnedToCore(motorUpdateTask, "MotorUpdateTask", 2048, NULL, 3, &motorUpdateTaskHandle, 1);  // Core 0
    esp_task_wdt_add(motorUpdateTaskHandle);                                                                // Register with WDT
}

void loop()
{
    // Print motor status every 2 seconds
    static uint32_t lastStatusTime = 0;
    if (millis() - lastStatusTime >= 2000)
    {
        lastStatusTime = millis();
    }

    esp_task_wdt_reset();
    vTaskDelay(1);
}

void motorUpdateTask(void* pvParameters)
{
    const uint8_t    MOTOR_UPDATE_TIME = 4;
    const TickType_t xFrequency        = pdMS_TO_TICKS(MOTOR_UPDATE_TIME);
    TickType_t       xLastWakeTime     = xTaskGetTickCount();
    uint8_t          i                 = 0;

    while (1)
    {
        if (driverManager.testConnection(i))
        {
            auto status = driverManager.getDriverStatus(i);
            Serial.printf("Motor %d Status:\n", i);
            Serial.printf("  Current: %d mA\n", status.current);
            Serial.printf("  Temperature: %d\n", status.temperature);

            // Get current speed based on motor index
            int16_t speed = 0;
            switch (i)
            {
                case 0:
                    speed = motor0.getCurrentSpeed();
                    break;
                case 1:
                    speed = motor1.getCurrentSpeed();
                    break;
                case 2:
                    speed = motor2.getCurrentSpeed();
                    break;
                case 3:
                    speed = motor3.getCurrentSpeed();
                    break;
            }
            Serial.printf("  Speed: %d\n", speed);
        }

        esp_task_wdt_reset();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
