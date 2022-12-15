#pragma once

// definicja stałej odpowiadającej za wykluczenie niektórych interfejsów Win32API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// definicja stałej odpowiadającej za ignorowanie przestarzałych interfejsów
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <string>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // zalinkowanie biblioteki WinSock

namespace SOCK_NET
{
	enum class Action // typ wyliczeniowy reprezentujący akcję klienta
	{
		HEADER, // wysyła nagłówek
		SEND, // przesyła plik do serwera (serwer odbiera)
		RECV, // odbiera plik z serwera (klient odbiera)
	};

	// klasa reprezentującą cześć wspólną klienta i serwera
	class Lib
	{
		private:
			WSADATA wsaData; // struktura wsaData
			WORD reqVersion = MAKEWORD(2, 2); // wymagana wersja WinSock

			const char* address = "127.0.0.1"; // adres serwera
			const int port = 6969; // port serwera

		public:
			static const int MAX_BUFF = 1; // maksymalny bufor na nagłówek
			static const int FRAME_BUFF = 1024; // bufor na dane (odczyt/zapis pliku)
			static const std::string DELIMITER; // domyślny delimiter rozdzielający akcję od nazwy pliku
			static const std::string SEND_INDC; // sekwencja znaków po której następuje wysłanie danych
			static const std::string FILE_SEP; // separator pliku zapisywanego przez serwer

			~Lib(); // destruktor zwalaniający zasoby biblioteki WinSock
			bool startWinsock(); // inicjalizacja biblioteki WinSock
			void closeWinsock(); // zwalnianie zasobów biblioteki WinSock
			void log(); // funkcja logger wyświetlająca komunikaty serwera/klienta

			const char* getAddress() const; // getter zwracający adres
			const int getPort() const; // getter zwracający port
	};
}
