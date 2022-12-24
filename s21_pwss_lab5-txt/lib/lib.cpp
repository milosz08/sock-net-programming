// biblioteka - implementacja klasy
#include "lib.h";

SOCK_NET::Lib::Lib() : address("127.0.0.1"), port(8080)
{
}

SOCK_NET::Lib::~Lib()
{
}

bool SOCK_NET::Lib::startWinsock()
{
	// inicjalizacja biblioteki WinSock, jeśli nie zdoła zainicjalizować, error w konsoli i 
	// zwraca false
	if (WSAStartup(this->reqVersion, &this->wsaData) != 0)
	{
		this->log(ERROR_L, __FILE__, __LINE__, "Inicjalizacja biblioteki WinSock zakonczona niepowodzeniem.");
		return false;
	}
	// sprawdź, czy wybrana wersja WinSock jest zgodna z wersją 2.2
	if (LOBYTE(this->wsaData.wVersion) != 2 || HIBYTE(this->wsaData.wVersion) != 2)
	{
		this->log(ERROR_L, __FILE__, __LINE__, "Nie znaleziono wymaganej wersji biblioteki WinSock.");
		this->closeWinsock(); // zwolnij zasoby
		return false;
	}
	this->log(INFO_L, __FILE__, __LINE__, "Inicjalizacja biblioteki WinSock 2.2 zakonczona pomyslnie.");
	return true; // jeśli wszystko ok, zwróć true
}

void SOCK_NET::Lib::closeWinsock()
{
	this->log(INFO_L, __FILE__, __LINE__, "Konczenie dzialania i zwolnienie zasobow WinSock.");
	WSACleanup(); // zwolnienie zasobów zaalokowanych na bibliotekę WinSock
}

void SOCK_NET::Lib::insertConnParamArgs(int& argc, char** argv)
{
	bool foundAddress = false; // flaga informująca o pobraniu adresu z argumentów
	bool foundPort = false; // flaga informująca o pobraniu portu z argumentów
	if (argc < 3) return; // jeśli nie podano 2 argumentów, zakończ działanie metody
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
	if (!foundAddress) this->log(ERROR_L, __FILE__, __LINE__, "Nie znaleziono oczekiwanego parametru '--address=0.0.0.0'.");
	else this->log(INFO_L, __FILE__, __LINE__, "Wczytano parametr wejsciowy '--address=" + this->address + "'.");

	if (!foundPort) this->log(ERROR_L, __FILE__, __LINE__, "Nie znaleziono oczekiwanego parametru '--port=0000'.");
	else this->log(INFO_L, __FILE__, __LINE__, "Wczytano parametr wejsciowy '--port=" + std::to_string(this->port) + "'.");
}

void SOCK_NET::Lib::log(const std::string& level, const std::string file, const int line,
	const std::string& msg, bool EOL, std::string preMsg)
{
	time_t time = std::time(nullptr); // zwróć strukturę czasu
	tm localTime = *std::localtime(&time); // pobierz wartość lokalnego czasu
	// nazwa pliku pobrana z całej ścieżki
	std::string fileName = std::filesystem::path(file).filename().string();
	// wydrukuj na ekran konsoli zawartość komunikatu
	std::cout << "[" << std::put_time(&localTime, "%d-%m-%Y %H:%M:%S") << "] [" << level << "] ["
		<< fileName << ":" << line << "]" << (preMsg != "" ? " " + preMsg : "") << "\t : " 
		<< msg << (EOL ? "\n" : "");
}

const char* SOCK_NET::Lib::getAddress() const
{
	return this->address.c_str(); // zwraca adres serwera (w postaci ciągu znaków)
}

const u_short SOCK_NET::Lib::getPort() const
{
	return this->port; // zwraca port serwera (w postaci liczby całkowitej)
}