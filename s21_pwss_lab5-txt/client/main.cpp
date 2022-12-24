// program klienta - plik main
#include "client.h"

int main(int argc, char** argv)
{
	SOCK_NET::Lib lib; // obiekt klasy współdzielonej
	// inicjalizacja biblioteki WinSock, jeśli się nie powiedzie zakończ program
	if (!lib.startWinsock()) return 1;

	lib.insertConnParamArgs(argc, argv); // wczytanie adresu i portu z argumentów wejściowych

	// inicjalizacja obiektu klienta z adresem IP i portem z klasy Lib
	SOCK_NET::Client client(lib.getAddress(), lib.getPort());

	// nawiąż połączenie z serwerem, jeśli błąd koniec działania
	if (client.createConnection())
	{
		client.selectAction(); // wybranie akcji (wysyłanie/odbieranie pliku)
		// wysyłanie nagłówka z informacją o akcji i pliku, jeśli blad nie pobieraj akcji
		if (client.sendHeader())
		{
			if (client.getAction() == SOCK_NET::Action::SEND) client.sendFile();
			else client.receiveFile();
		}
	}

	client.closeConnection(); // zwolnij połączenie jeśli takowe istnieje
	lib.closeWinsock(); // zwolnij zasoby biblioteki WinSock
	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}