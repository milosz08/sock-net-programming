// biblioteka - implementacja klasy
#include "lib.h";

const char SOCK_NET::Lib::DELIMITER = '|';
const char SOCK_NET::Lib::SEND_INDC = '\n';
const char SOCK_NET::Lib::FILE_SEP = '_';

bool SOCK_NET::Lib::startWinsock()
{
	// inicjalizacja biblioteki WinSock, jeśli nie zdoła zainicjalizować, error w konsoli i 
	// zwraca false
	if (WSAStartup(this->reqVersion, &this->wsaData) != 0)
	{
		std::cerr << "[ERROR::LIB]\tInicjalizacja biblioteki WinSock zakonczona niepowodzeniem.\n";
		return false;
	}
	// sprawdź, czy wybrana wersja WinSock jest zgodna z wersją 2.2
	if (LOBYTE(this->wsaData.wVersion) != 2 || HIBYTE(this->wsaData.wVersion) != 2)
	{
		std::cerr << "[ERROR::LIB]\tNie znaleziono wymaganej wersji biblioteki WinSock.\n";
		this->closeWinsock(); // zwolnij zasoby
		return false;
	}
	std::cout << "[INFO::LIB]\tInicjalizacja biblioteki WinSock 2.2 zakonczona pomyslnie.\n";
	return true; // jeśli wszystko ok, zwróć true
}

SOCK_NET::Lib::~Lib()
{
	this->closeWinsock(); // zwolnienie zasobów zaalokowanych na bibliotekę
}

void SOCK_NET::Lib::closeWinsock()
{
	std::cout << "[INFO::LIB]\tKonczenie dzialania i zwolnienie zasobow WinSock.\n";
	WSACleanup(); // zwolnienie zasobów zaalokowanych na bibliotekę WinSock
}

const char* SOCK_NET::Lib::getAddress() const
{
	return this->address; // zwraca adres serwera (w postaci ciągu znaków)
}

const int SOCK_NET::Lib::getPort() const
{
	return this->port; // zwraca port serwera (w postaci liczby całkowitej)
}