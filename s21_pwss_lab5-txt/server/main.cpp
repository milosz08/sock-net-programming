// program serwera - plik main
#include "server.h"

int main(int argc, char** argv)
{
	SOCK_NET::Lib* lib = new SOCK_NET::Lib; // obiekt klasy współdzielonej
	// inicjalizacja biblioteki WinSock, jeśli się nie powiedzie zakończ program
	if (!lib->startWinsock())
	{
		delete lib; // usunięcie obiektu klasy współdzielonej
		return 1;
	}

	// stworzenie obiektu serwera z adresem i portem z klasy Lib
	SOCK_NET::Server* server = new SOCK_NET::Server(lib->getAddress(), lib->getPort());

	// zainicjalizuj serwer, jeśli błąd koniec działania, jeśli nie uruchom główną pętlę
	if (server->initialize()) server->mainLoop();

	delete server; // usunięcie obiektu serwera i sprzątanie
	delete lib; // usunięcie obiektu klasy współdzielonej

	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}