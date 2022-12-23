// program klienta - plik main
#include "client.h"

int main(int argc, char** argv)
{
	SOCK_NET::Lib lib; // stworzenie obiektu biblioteki
	if (!lib.startWinsock()) return 1; // jeśli nie uda się zainicjalizować, zakończ
	lib.insertConnParamArgs(argc, argv); // wprowadzenie parametrów połączenia z argumentów
	
	SOCK_NET::Client client(lib.getAddress(), lib.getPort()); // stworzenie obiektu klienta

	if (client.createConnection()) // zainicjalizuj połączenie z serwerem, jeśli się nie uda zakończ
	{
		client.insertResourceName(); // wprowadzenie nazwy zasobu
		client.insertHeaderParams(); // wprowadzenie parametrów GET zapytania
		// kompletowanie i wysyłanie nagłówka, jeśli się nie powiedzie nie pobieraj danych
		if (client.completeAndSendHeader()) client.receiveAndDecodeToken();
	}

	client.closeConnection(); // zamknięcie połączenia, jeśli takowe istnieje
	lib.closeWinsock(); // zwolnienie zasobów biblioteki WinSock
	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}