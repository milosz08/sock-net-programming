// plik main.h
#pragma once

// definicja stałej odpowiadającej za wykluczenie niektórych interfejsów Win32API.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // zalinkowanie biblioteki

void rawSingleAddress(const char* address);
void domainMultipleAddress(const char* address, const char* port);
int main(int argc, char** argv);