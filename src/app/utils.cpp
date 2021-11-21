#include "utils.h"
#include "app/app.h"

// ************* helpers
void handleError(String message, String system)
{
    terminal.printMessage(TerminalMessage(message, system, ERROR, micros()));

    esp.uart0.print("\n\nSystem Aborted. Restart\n\n");
    while (1)
    {
        pixels.setPixelColor(0, pixels.Color(50, 0, 0));
        pixels.show();
        delay(250);
    }
}

uint8_t cpu0_index = 0;
uint8_t cpu1_index = 0;
TerminalMessage message_queue_memory_cpu0[10];
TerminalMessage message_queue_memory_cpu1[10];

void printMessage(TerminalMessage &message, uint16_t wait_ticks)
{
    xSemaphoreTake(app.debug_message_queue_mutex, wait_ticks);
    xQueueSend(app.debug_message_queue, (void *)&message, 0);
    xSemaphoreGive(app.debug_message_queue_mutex);
}

void terminalOutput(void *parameters)
{
    // Local Variables
    TerminalMessage debug_message;
    String file_content;

    while (1)
    {
        // * Print message to console
        if (xQueueReceive(app.debug_message_queue, (void *)&debug_message, 1) == pdTRUE)
        {
            terminal.printMessage(debug_message);
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}