#include "wsapi.h"

// domyślny konstrutor, przypisujący do adresu i portu serwera domyślne wartości
SOCK_NET::WsAPI::WsAPI() : address(DEF_HOST), port(DEF_PORT)
{
}

SOCK_NET::WsAPI::~WsAPI()
{
	closeWinsock();
}

bool SOCK_NET::WsAPI::startWinsock()
{
	// inicjalizacja biblioteki WinSock, jeśli nie zdoła zainicjalizować, error w konsoli i 
	// zwraca false
	if (WSAStartup(this->reqVersion, &this->wsaData) != 0)
	{
		this->logError("Inicjalizacja biblioteki WinSock zakonczona niepowodzeniem", __FILE__, __LINE__);
		return false;
	}
	// sprawdź, czy wybrana wersja WinSock jest zgodna z wersją 2.2
	if (LOBYTE(this->wsaData.wVersion) != 2 || HIBYTE(this->wsaData.wVersion) != 2)
	{
		this->logError("Nieprawidlowa wersja biblioteki WinSock", __FILE__, __LINE__);
		this->closeWinsock(); // zwolnij zasoby
		return false;
	}
	this->logInfo("Inicjalizacja biblioteki WinSock 2.2 zakonczona pomyslnie", __FILE__, __LINE__);
	return true; // jeśli wszystko ok, zwróć true
}

void SOCK_NET::WsAPI::closeWinsock()
{
	// zwolnienie zasobów zaalokowanych na bibliotekę WinSock, jeśli takowe istnieją
	if (&this->wsaData != nullptr)
	{
		this->logInfo("Zwolnienie zasobow biblioteki WinSock", __FILE__, __LINE__);
		WSACleanup();
	}
}

void SOCK_NET::WsAPI::insertConnParamArgs(int& argc, char** argv)
{
	bool foundAddress = false; // flaga informująca o pobraniu adresu z argumentów
	bool foundPort = false; // flaga informująca o pobraniu portu z argumentów
	// jeśli nie podano 2 argumentów, zakończ działanie metody
	if (argc < 3)
	{
		this->logWarn("Nie znaleziono argumentow wejsciowych", __FILE__, __LINE__);
		this->logInfo("Domyslne parametry polaczenia: " + DEF_HOST + ":" + std::to_string(DEF_PORT), __FILE__, __LINE__);
		return;
	}
	for (int i = 0; i < argc; i++) // przejdź przez wszystkie podane argumenty
	{
		std::string param = argv[i]; // przypisz do stringa aby móc używać jego metod
		int separatorLocation = param.find('='); // znajdź separator w argumencie
		if (separatorLocation < 0) continue; // jeśli nie znajdzie, przejdź kolejną iterację

		std::string key = param.substr(0, separatorLocation); // klucz
		std::string value = param.substr(separatorLocation + 1, param.size()); // wartość
		if (key == "--address") // jeśli podany parametr to adres, przypisz do pola
		{
			this->address = value;
			foundAddress = true; // ustaw flagę na znaleziony
		}
		else if (key == "--port") // jeśli podany parametr to port, skonwertuj i przypisz do pola 
		{
			this->port = std::stoi(value);
			foundPort = true; // ustaw falgę na znaleziony
		}
	}
	// wypisz komunikaty na ekran konsoli o rezultacie przetwarzania argumentów wejściowych
	if (!foundAddress) this->logError("Nie znaleziono oczekiwanego argumentu '--address", __FILE__, __LINE__);
	else this->logInfo("Wczytano argument wejsciowy '--address=" + this->address + "'", __FILE__, __LINE__);

	if (!foundPort) this->logError("Nie znaleziono oczekiwanego argumentu '--port'", __FILE__, __LINE__);
	else this->logInfo("Wczytano argument wejsciowy '--port=" + std::to_string(this->port) + "'", __FILE__, __LINE__);
}

void SOCK_NET::WsAPI::logInfo(const std::string message, const std::string file, const int line)
{
	log(INFO_L, message, file, line);
}

void SOCK_NET::WsAPI::logWarn(const std::string message, const std::string file, const int line)
{
	log(WARN_L, message, file, line);
}

void SOCK_NET::WsAPI::logError(const std::string message, const std::string file, const int line)
{
	log(ERROR_L, message, file, line);
}

bool SOCK_NET::WsAPI::checkIfValueIsDouble(double from, double to, std::string value, double* out)
{
	try
	{
		double convertedValue = std::stod(value); // konwertuj ciąg znaków na double
		// sprawdź czy mieści się w zadanym przedziale, jeśli nie zwróć wartość domyślną
		if (convertedValue < from || convertedValue > to) return false;
		*out = convertedValue; // przypisz do wyłuskanej wartości ze wskaźnika sparsowaną wartość
		return true; // jeśli wyszystko git, zwróć true
	}
	catch (std::invalid_argument& err)
	{
		return false; // w przypadku złapania wyjątku, zwróć wartość domyślną
	}
}

void SOCK_NET::WsAPI::log(const std::string& level, const std::string message, const std::string file, const int line)
{
	time_t time = std::time(nullptr); // zwróć strukturę czasu
	tm localTime = *std::localtime(&time); // pobierz wartość lokalnego czasu
	// nazwa pliku pobrana z całej ścieżki
	std::string inputTraceFilename = file.substr(file.find_last_of("/\\") + 1);
	// wydrukuj na ekran konsoli zawartość komunikatu
	std::string nameWithLine = inputTraceFilename + ":" + std::to_string(line);
	std::cout
		<< "[" << std::put_time(&localTime, "%d-%m-%Y %H:%M:%S") << "] "
		<< "[" << std::left << std::setw(5) << level << "] "
		<< "[" << std::left << std::setw(22) << nameWithLine << "]"
		<<  ": " << message << '\n';
}

const std::string SOCK_NET::WsAPI::getAddress() const
{
	return this->address; // zwraca adres serwera (w postaci ciągu znaków)
}

const u_short SOCK_NET::WsAPI::getPort() const
{
	return this->port; // zwraca port serwera (w postaci liczby całkowitej)
}
