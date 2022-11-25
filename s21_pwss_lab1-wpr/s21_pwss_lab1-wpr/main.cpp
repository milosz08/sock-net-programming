// plik main.cpp
#include "main.h"

int main(int argc, char** argv)
{
	WSADATA wsaData; // inicjalizacja struktury biblioteki WinSock
	// zapełnienie zainicjalizowanej struktury danymi i uruchomienie WinSock na wersji 2
	// jeśli nie uda się uruchomić, zwróci wartość różną od 0 i komunikat błędu
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Uruchomienie zakonczone niepowodzeniem.\n";
		return 1;
	}

	rawSingleAddress("192.168.100.1"); // umieść w strukturze sockaddr adres IP
	std::cout << std::endl;
	// umieść w strukturze adres domenowy i wypisz skojarzone z nim adresy IP
	domainMultipleAddress("polsl.pl", "80");

	WSACleanup(); // zwolnienie zasobów
	return 0;
}

// funkcja odpowiadająca za zapełnianie struktury sockaddr_in przy użyciu standardowego zapisu adresu IPv4
void rawSingleAddress(const char* address)
{
	struct sockaddr_in sa; // inicjalizacja struktury
	// tablica znaków do której funkcja inet_ntop wpisuje przekonwertowany adres IP z postaci binarnej
	char ipstr[INET_ADDRSTRLEN];

	// zapełnij strukturę sin_addr znajdującą się w sockaddr_in przy użyciu adresu IPv4
	// dostarczanego poprzez parametr funkcji w przypadku błędu, zwróci wartość mniejszą od 1 i wyświetli komunikat.
	if ((inet_pton(AF_INET, address, &(sa.sin_addr)) < 1))
	{
		std::cerr << "Adres: " << address << " nie jest prawidlowym adresem IPv4.\n";
		return;
	}

	// rzutowanie adresu struktury sa na wskaźnik typu sockaddr_in
	struct sockaddr_in* ipv4 = (struct sockaddr_in*)&sa;

	// konwersja adresu IP z postaci binarnej pobieranej ze struktury ipv4 na podstać znakową
	// jeśli nie zdoła wykonać konwersji, zwróci NULL, po czym wypisze komunikat
	if ((inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof ipstr) == NULL))
	{
		std::cerr << "Nieudane odczytanie adresu IPv4 ze struktury sockaddr_in.\n";
		return;
	}
	std::cout << "Adres IPv4 uzyskany przy odczycie struktury: " << ipstr << "\n";
}

// funkcja odpowiadająca za zapełnianie struktury sockaddr_in przy użyciu adresu domeny oraz
// wyświetlanie przypisanych do tej domeny adresów IP oraz wyświetlanie portu
void domainMultipleAddress(const char* address, const char* port = "80")
{
	struct addrinfo hints, * res; // deklaracja struktury podpowiedzi (hints) oraz odpowiedzi
	char ipstr[INET_ADDRSTRLEN]; // tablica znaków przechowująca adres IP w postaci tekstowej
	// status zwrócony po próbie pobrania struktury addrinfo przy użyciu
	// funkcji getaddrinfo()	
	int status;

	memset(&hints, 0, sizeof(hints)); // zapełnianie struktury hints zerami
	// sparametryzowanie odpowiedzi przydzielonych adresów IP do adresu domenowego i 
	// odfiltrowanie tylko takich, które spełniają poniższe założenia 
	hints.ai_family = AF_INET; // preferowany typ adresu IP (IPv4)
	hints.ai_socktype = SOCK_STREAM; // preferowany typ gniazda (TCP)
	hints.ai_flags = AI_PASSIVE; // wypełnienie adresu IP w strukturze

	// pobierz wszystkie przypisane adresy (zapisane w strukturach sockaddr_in) do wybranego 
	// adresu domenowego, jeśli pobranie się nie powiedzie, zwróci wartość różną od zera oraz wyświetli komunikat błędu
	if ((status = getaddrinfo(address, port, &hints, &res)) != 0)
	{
		std::cerr << "Nieudane dodanie adresow do struktury addrinfo. Blad: " << gai_strerror(status) << "\n";
		freeaddrinfo(res); // zwolnienie zasobów zaalokowanych na strukturę res
		return;
	}

	std::cout << "Uzyskane adresy IPv4 z domeny " << address << ":\n";
	// przejdź przez wszystkie struktury przechowujące przypisane adresy IPv4 do adresu 
	// domenowego
	for (struct addrinfo* p = res; p != NULL; p = p->ai_next)
	{
		// rzutowanie adresu struktury sa na wskaźnik typu sockaddr_in
		struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

		// konwersja adresu IP z postaci binarnej pobieranej ze struktury ipv4 na podstać 
		// znakową, jeśli nie zdoła wykonać konwersji, zwróci NULL, po czym wypisze komunikat
		if ((inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof ipstr) == NULL))
		{
			std::cerr << "Nieudane dodanie adresu " << &(ipv4->sin_addr) << " do struktury sockaddr_in.";
			freeaddrinfo(res); // zwolnienie zasobów zaalokowanych na strukturę res
			return;
		}
		// funkcja htons konwertująca port w zapisie binarnym na zapis znakowy
		std::cout << "IPv4: " << ipstr << ", port: " << htons(ipv4->sin_port) << "\n";
	}
	freeaddrinfo(res); // zwolnienie zasobów zaalokowanych na strukturę res
}