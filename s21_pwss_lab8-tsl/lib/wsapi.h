// klasa inicjalizacji WinsockAPI - sygnatura klasy
#ifndef WSAPI_H
#define WSAPI_H

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
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // zalinkowanie biblioteki WinSock

namespace SOCK_NET
{
	// stałe dla loggera
	const std::string INFO_L = "INFO";
	const std::string WARN_L = "WARN";
	const std::string ERROR_L = "ERROR";

	// domyślne parametry serwera przypisywane w przypadku niepodania argumentów wejściowych
	const std::string DEF_HOST = "127.0.0.1";
	const u_short DEF_PORT = 8080;

	class WsAPI
	{
	private:
		WSADATA wsaData;
		WORD reqVersion = MAKEWORD(2, 2);
		std::string address;
		u_short port;

	public:
		WsAPI(const WsAPI&) = delete; // usunięcie konstruktora kopiującego
		WsAPI(WsAPI&&) = delete; // usunięcie konstruktora przenoszącego
		WsAPI(); // konstruktor domyślny bezparametrowy
		virtual ~WsAPI(); // destruktor sprzątający

		bool startWinsock(); // inicjalizacja biblioteki WinSock
		void closeWinsock(); // zwalnianie zasobów biblioteki WinSock
		void insertConnParamArgs(int& argc, char** argv); // wprowadź parametry wejściowe połączenia

		WsAPI& operator= (const WsAPI&) = delete; // usunięcie operatora przypisania (kopiowanie)
		WsAPI& operator= (WsAPI&&) = delete; // usunięcie operatora przypisania (przenoszenie)

		const std::string getAddress() const; // getter zwracający adres
		const u_short getPort() const; // getter zwracający port

		static void logInfo(const std::string message, const std::string file, const int line);
		static void logWarn(const std::string message, const std::string file, const int line);
		static void logError(const std::string message, const std::string file, const int line);

		// konwersja string na double z dodatkowym sprawdzaniem zakresu
		static bool checkIfValueIsDouble(double from, double to, std::string value, double* out);

	private:
		// logowanie danych w konsoli
		static void log(const std::string& level, const std::string message, const std::string file, const int line);
	};
}

#endif // WSAPI_H