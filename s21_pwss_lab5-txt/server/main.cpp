// program serwera - plik main
#include "server.h"

int main(int argc, char** argv)
{
	SOCK_NET::Lib lib; // obiekt klasy współdzielonej
	// inicjalizacja biblioteki WinSock, jeśli się nie powiedzie zakończ program
	if (!lib.startWinsock()) return 1;

	lib.insertConnParamArgs(argc, argv); // wczytanie adresu i portu z argumentów wejściowych

	// stworzenie obiektu serwera z adresem i portem z klasy Lib
	SOCK_NET::Server server(lib.getAddress(), lib.getPort());

	// zainicjalizuj serwer, jeśli błąd koniec działania, jeśli nie uruchom główną pętlę
	if (server.initialize()) server.mainLoop();

	server.closeAllConnections(); // zamknij wszystkie aktywne połączenia klientów i serwera
	lib.closeWinsock(); // zwolnij zasoby biblioteki WinSock
	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}