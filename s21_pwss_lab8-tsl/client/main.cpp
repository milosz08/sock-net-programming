// program klienta - plik main
#include "client.h"

int main(int argc, char** argv)
{
	SOCK_NET::WsAPI wsApi; // obiekt klasy współdzielonej
	// inicjalizacja biblioteki WinSock, jeśli się nie powiedzie zakończ program
	if (!wsApi.startWinsock()) return -1;

	wsApi.insertConnParamArgs(argc, argv); // wczytanie adresu i portu z argumentów wejściowych
	SOCK_NET::Client client(wsApi.getAddress(), wsApi.getPort()); // stworzenie instancji klienta
	client.insertLocationFromParamArgs(argc, argv); // pobierz długość i szerokość geogr.

	// wczytanie ścieżek do certyfikatów OpenSSL, jeśli się nie powiedzie zakończ program
	if (!client.insertSslContextArgs(argc, argv)) return -2;
	// zainicjalizowanie certyfikatów klienta, jeśli się nie powiedzie zakończ program
	if (!client.initializeSsl()) return -3;
	// nawiąż połączenie z serwerem, jeśli się nie powiedzie zakończ program
	if (!client.connectWithServer()) return -4;
	// pozyskaj dane z API i wyślij do serwera.
	if (!client.sendDataToServer()) return -5;

	return 0;
}