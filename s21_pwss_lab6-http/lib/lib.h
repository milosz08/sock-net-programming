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
#include <iomanip>
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

	const size_t FRAME_BUFF = 4096; // rozmiar ramki

	// klasa reprezentującą bibliotekę, oznaczona jako finalna w celu zablokowania dziedziczenia
	class Lib final
	{
		private:
			WSADATA wsaData; // struktura wsaData
			WORD reqVersion = MAKEWORD(2, 2); // wymagana wersja WinSock
			std::string address; // adres zapisany w postaci 0.0.0.0
			u_short port; // port zapisany w postaci 0000

			static bool isBase64(u_char c); // sprawdzenie, czy znak jest zakodowany w Base64

		public:
			explicit Lib(const Lib&) = delete; // usunięcie konstruktora kopiującego
			explicit Lib(Lib&&) = delete; // usunięcie konstruktora przenoszącego
			explicit Lib(); // konstruktor domyślny bezparametrowy
			virtual ~Lib(); // wirtualny destruktor

			bool startWinsock(); // inicjalizacja biblioteki WinSock
			void closeWinsock(); // zwolnienie zasobów WinSock
			void insertConnParamArgs(int& argc, char** argv); // wprowadź parametry wejściowe połączenia

			// metoda dekodująca ciąg znaków zakodowany w Base64
			// źródło: https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
			static std::vector<u_char> base64decode(std::string const& encodedString);
			
			// drukowanie na ekran konsoli logów
			static void log(const std::string& level, const std::string file, const int line, const std::string& msg);

			Lib& operator= (const Lib&) = delete; // usunięcie operatora przypisania (kopiowanie)
			Lib& operator= (Lib&&) = delete; // usunięcie operatora przypisania (przenoszenie)

			const char* getAddress() const; // getter zwracający adres
			const u_short getPort() const; // getter zwracający port
	};
}

#endif // LIB_H