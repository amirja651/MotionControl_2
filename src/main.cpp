#include "SystemDiagnostics.h"
#include <Arduino.h>
#include <SPI.h>
#include <SimpleCLI.h>
#include <TMCStepper.h>
#include <esp_task_wdt.h>

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
}

void loop()
{
    esp_task_wdt_reset();
    vTaskDelay(100);
}
