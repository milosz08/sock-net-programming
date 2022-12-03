// receiver.h
#pragma once

// definicja stałej odpowiadającej za wykluczenie niektórych interfejsów Win32API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// definicja stałej odpowiadającej za ignorowanie przestarzałych interfejsów
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#define PORT 6969 // port
#define FRAME_BUFF 10 // rozmiar bufora (ramki) w bajtach

const char* RECV_INET = "127.0.0.1"; // adres serwera
// nazwa dobieranego pliku od klienta
const char* FILE_NAME = "test.txt";

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")  // zalinkowanie biblioteki

void receiveFile();
int main(int argc, char** argv);