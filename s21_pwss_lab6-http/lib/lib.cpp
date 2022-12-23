// biblioteka - implementacja klasy
#include "lib.h"

SOCK_NET::Lib::Lib() : address("127.0.0.1"), port(8080)
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
	WSACleanup(); // zwolnienie zasobów biblioteki
	this->log(INFO_L, __FILE__, __LINE__, "Konczenie dzialania i zwolnienie zasobow WinSock.");
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

bool SOCK_NET::Lib::isBase64(u_char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<u_char> SOCK_NET::Lib::base64decode(std::string const& encodedString)
{
	const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int inLen = encodedString.size();
	int i = 0;
	int in_ = 0;
	u_char charArray4[4], charArray3[3];
	std::vector<u_char> ret;
	while (inLen-- && (encodedString[in_] != '=') && isBase64(encodedString[in_]))
	{
		charArray4[i++] = encodedString[in_]; in_++;
		if (i == 4)
		{
			for (i = 0; i < 4; i++) charArray4[i] = base64Chars.find(charArray4[i]);
			charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
			charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
			charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];
			for (i = 0; (i < 3); i++) ret.push_back(charArray3[i]);
			i = 0;
		}
	}
	if (i)
	{
		for (int j = i; j < 4; j++) charArray4[j] = 0;
		for (int j = 0; j < 4; j++) charArray4[j] = base64Chars.find(charArray4[j]);
		charArray3[0] = (charArray4[0] << 2) + ((charArray4[1] & 0x30) >> 4);
		charArray3[1] = ((charArray4[1] & 0xf) << 4) + ((charArray4[2] & 0x3c) >> 2);
		charArray3[2] = ((charArray4[2] & 0x3) << 6) + charArray4[3];
		for (int j = 0; (j < i - 1); j++) ret.push_back(charArray3[j]);
	}
	return ret;
}

void SOCK_NET::Lib::log(const std::string& level, const std::string file, const int line, const std::string& msg)
{
	time_t time = std::time(nullptr); // zwróć strukturę czasu
	tm localTime = *std::localtime(&time); // pobierz wartość lokalnego czasu
	// nazwa pliku pobrana z całej ścieżki
	std::string fileName = std::filesystem::path(file).filename().string();
	// wydrukuj na ekran konsoli zawartość komunikatu
	std::cout << "[" << std::put_time(&localTime, "%d-%m-%Y %H:%M:%S") << "] [" << level << "] ["
			  << fileName << ":" << line << "]\t : " << msg << "\n";
}

const char* SOCK_NET::Lib::getAddress() const
{
	return this->address.c_str(); // zwróć adres jako tablicę znaków
}

const u_short SOCK_NET::Lib::getPort() const
{
	return this->port; // zwróć port
}