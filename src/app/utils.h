#pragma once

#include <app/som/som.h>

void handleError(String message, String system);
void printMessage(TerminalMessage &message, uint16_t port_ticks = 0);
void terminalOutput(void *parameters);