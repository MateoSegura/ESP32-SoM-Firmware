#include "utils.h"

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

void printMessage(String message, String system)
{
    terminal.printMessage(TerminalMessage(message, system, INFO, micros()));
}