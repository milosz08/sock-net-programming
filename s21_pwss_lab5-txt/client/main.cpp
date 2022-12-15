// program klienta - plik main
#include "client.h"

int main(int argc, char** argv)
{
	SOCK_NET::Lib* lib = new SOCK_NET::Lib; // obiekt klasy współdzielonej
	// inicjalizacja biblioteki WinSock, jeśli się nie powiedzie zakończ program
	if (!lib->startWinsock())
	{
		delete lib; // usunięcie obiektu klasy współdzielonej (i zwolnienie zasobów)
		return 1;
	}

	// inicjalizacja obiektu klienta z adresem IP i portem z klasy Lib
	SOCK_NET::Client* client = new SOCK_NET::Client(lib->getAddress(), lib->getPort());

	// nawiąż połączenie z serwerem, jeśli błąd koniec działania
	if (client->createConnection())
	{
		client->selectAction(); // wybranie akcji (wysyłanie/odbieranie pliku)
		// wysyłanie nagłówka z informacją o akcji i pliku, jeśli blad nie pobieraj akcji
		if (client->sendHeader())
		{
			if (client->getAction() == SOCK_NET::Action::SEND) client->sendFile();
			else client->receiveFile();
		}
	}

	delete client; // usunięcie obiektu klienta (i zwolnienie zasobów)
	delete lib; // usunięcie obiektu klasy współdzielonej (i zwolnienie zasobów)

	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}