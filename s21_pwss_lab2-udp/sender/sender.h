#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#define PORT 6969
#define FRAME_BUFF 64000

const char* RECV_INET = "127.0.0.1";
const char* FILE_NAME = "VirtualBox-6.1.4-136177-Win.exe";

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void sendFile();
int main(int argc, char** argv);