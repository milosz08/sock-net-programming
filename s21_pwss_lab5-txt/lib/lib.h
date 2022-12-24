// biblioteka - sygnatura klasy
#ifndef LIB_H
#define LIB_H

// definicja stałej odpowiadającej za wykluczenie niektórych interfejsów Win32API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// definicja stałej odpowiadającej za ignorowanie przestarzałych interfejsów
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <ctime>
#include <string>
#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib") // zalinkowanie biblioteki WinSock

namespace SOCK_NET
{
	// stałe dla funkcji loggera
	const std::string INFO_L = "INFO";
	const std::string ERROR_L = "ERROR";
	const std::string WARN_L = "WARN";

	enum class Action // typ wyliczeniowy reprezentujący akcję klienta
	{
		HEADER, // wysyła nagłówek
		SEND, // przesyła plik do serwera (serwer odbiera)
		RECV, // odbiera plik z serwera (klient odbiera)
	};

	const int MAX_BUFF = 10; // maksymalny bufor na nagłówek
	const int FRAME_BUFF = 1024; // bufor na dane (odczyt/zapis pliku)
	const char DELIMITER = '|'; // domyślny delimiter rozdzielający akcję od nazwy pliku
	const char SEND_INDC = '\n'; // sekwencja znaków po której następuje wysłanie danych
	const char FILE_SEP = '_'; // separator pliku zapisywanego przez serwer

	// klasa reprezentującą cześć wspólną klienta i serwera
	class Lib final
	{
		private:
			WSADATA wsaData; // struktura wsaData
			WORD reqVersion = MAKEWORD(2, 2); // wymagana wersja WinSock

			std::string address; // adres serwera
			u_short port; // port serwera

		public:
			explicit Lib(const Lib&) = delete; // usunięcie konstruktora kopiującego
			explicit Lib(Lib&&) = delete; // usunięcie konstruktora przenoszącego
			explicit Lib(); // konstruktor domyślny bezparametrowy
			virtual ~Lib(); // destruktor wirtualny zwalaniający zasoby biblioteki WinSock
			
			bool startWinsock(); // inicjalizacja biblioteki WinSock
			void closeWinsock(); // zwalnianie zasobów biblioteki WinSock
			void insertConnParamArgs(int& argc, char** argv); // wprowadź parametry wejściowe połączenia

			// drukowanie na ekran konsoli logów
			static void log(const std::string& level, const std::string file, const int line,
				const std::string& msg, bool EOL = true, std::string preMsg = "");

			Lib& operator= (const Lib&) = delete; // usunięcie operatora przypisania (kopiowanie)
			Lib& operator= (Lib&&) = delete; // usunięcie operatora przypisania (przenoszenie)

			const char* getAddress() const; // getter zwracający adres
			const u_short getPort() const; // getter zwracający port
	};
}

#endif // LIB_H