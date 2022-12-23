// program klienta - sygnatura klasy
#ifndef CLIENT_H
#define CLIENT_H

#define NOMINMAX // zapobieganie definiowaniu makr min i max 

#include <lib\lib.h> // zaimportowanie biblioteki
#include <map>

namespace SOCK_NET
{
	// klasa reprezentująca klienta, oznaczona jako finalna w celu zablokowania możliwości dziedziczenia
	class Client final
	{
		private:
			SOCKET sock = INVALID_SOCKET; // socket klienta (przypisywany w metodzie createConnection())
			const char* address; // adres do nawiązania połączenia
			const int port; // numer portu do nawiązania połączenia

			std::string resourceName; // nazwa zasobu pobieranego z serwera
			std::map<std::string, std::string> parameters; // parametry GET zapytania

		public:
			explicit Client(const Client&) = delete; // usunięcie konstruktora kopiującego
			explicit Client(Client&&) = delete; // usunięcie konstruktora przenoszącego
			explicit Client(); // konstruktor inicjalizujący klienta domyślnymi wartościami
			explicit Client(const char* address, const int& port); // konstruktor inicjalizujący klienta
			virtual ~Client(); // destruktor zwalniający zasoby

			Client& operator= (const Client&) = delete; // usunięcie operatora przypisania (kopiowanie)
			Client& operator= (Client&&) = delete; // usunięcie operatora przypisania (przenoszenie)

			bool createConnection(); // nawiązanie połączenia, przypisanie socketu do pola klasy
			void insertResourceName(); // wprowadzenie nazwy zasobu pobieranego z serwera
			void insertHeaderParams(); // wprowadzenie danych parametrów dołączanych do zasobu
			bool completeAndSendHeader(); // kompletowanie i wysyłanie nagłówka do serwera
			void receiveAndDecodeToken(); // odbieranie i dekodowanie tokenu z serwera
			void closeConnection(); // zamknięcie połączenia i zwolnienie zasobów
	};
}

#endif // CLIENT_H