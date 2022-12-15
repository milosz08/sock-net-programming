// program klienta - sygnatura klasy
#pragma once
#include <lib\lib.h> // import klasy współdzielonej

namespace SOCK_NET
{
	// klasa reprezentująca klienta, przechowująca metody umożliwiające wysłanie i pobranie
	// danych do i z serwera
	class Client
	{
		private:
			SOCKET sock = INVALID_SOCKET; // socket klienta (przypisywany w metodzie createConnection())
			const char* address; // adres do nawiązania połączenia
			const int port; // numer portu do nawiązania połączenia

			std::string inputStream; // ciąg wejściowy nagłówka wprowadzananego przez użytkownika
			std::string fileName; // nazwa pliku wysyłanego/odbieraneg
			SOCK_NET::Action action; // typ akcji (wysyłanie/odbieranie)

		public:
			Client(); // konstruktor inicjalizujący klienta domyślnymi wartościami
			Client(const char* address, const int& port); // konstruktor inicjalizujący klienta
			~Client(); // destruktor zwalniający zasoby

			bool createConnection(); // nawiązanie połączenie, przypisanie socketu do pola klasy
			void selectAction(); // wybranie akcji (pobieranie/wysyłanie)
			bool sendHeader(); // wysyłanie nagłówka do serwera z akcją (pobieranie/wysyłanie)
			void sendFile(); // metoda umożliwiająca wysyłanie pliku do serwera
			void receiveFile(); // metoda umożliwiająca odebranie pliku z serwera 
			void closeConnection(); // zamyka połączenie klienta i zwalnia zasoby

			SOCK_NET::Action getAction() const; // getter zwracający akcję wybraną przez użytkownika
	};
}