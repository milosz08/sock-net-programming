// program serwera - plik main
#include "server.h"

int main(int argc, char** argv)
{
	SOCK_NET::WsAPI wsApi; // obiekt klasy współdzielonej
	// inicjalizacja biblioteki WinSock, jeśli się nie powiedzie zakończ program
	if (!wsApi.startWinsock()) return -1;

	// wczytanie adresu i portu z argumentów wejściowych
	wsApi.insertConnParamArgs(argc, argv);
	SOCK_NET::Server server(wsApi.getAddress(), wsApi.getPort()); // stworzenie instancji serwera

	// wczytanie ścieżek do certyfikatów OpenSSL, jeśli się nie powiedzie zakończ program
	if (!server.insertSslContextArgs(argc, argv)) return -2;
	// zainicjalizowanie certyfikatów serwera, jeśli się nie powiedzie zakończ program
	if (!server.initializeSsl()) return -3;
	// zainicjalizowanie serwera, jeśli się nie powiedzie zakończ program
	if (!server.initializeServer()) return -4;
	server.receiveData(); // pobranie danych od klienta

	return 0;
}