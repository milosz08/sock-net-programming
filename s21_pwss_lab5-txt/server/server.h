// program serwera - sygnatura klasy
#ifndef SERVER_H
#define SERVER_H

#include <lib\lib.h>

#include <map>
#include <vector>

#define SHOW_HDR_PARTS 0 // dyrektywa preprocesora, jeśli ustawiona na 0 serwer pokazuje fragmenty nagłówka

namespace SOCK_NET
{
	// struktura przechowująca dane połaczonego klienta przechowywane w mapie, której
	// kluczem jest powiązany z tym klientem deskryptor socketu
	struct ClientData final
	{
		std::string address; // adres IP klienta
		int clNr = 0; // numer klienta (inkrementowana zmienna)
		Action action = Action::HEADER; // akcja wykonywana przez użytkownika
		FILE* fileHandler = nullptr; // uchwyt do pliku klienta
		std::string fileName; // nazwa pliku
		std::string header; // pełny header
		bool headerCollect = false; // nagłówek został w całości odebrany
		size_t sendBytesSize = 0; // całkowita ilość wysłanych bajtów
	};

	// klasa reprezentująca serwer umożliwiający łączenie się z wieloma klientami w trybie
	// nieblokującym i umożliwiającym odbieranie i wysyłanie plików
	class Server final
	{
		private:
			SOCKET sock = INVALID_SOCKET; // socket serwera inicjalizowany w metodzie initialize()
			const char* address = nullptr; // adres serwera
			const int port; // numer portu serwera
			std::vector<pollfd> descrs; // wektor deskryptorów socketów (serwer + klienci)
			std::map<SOCKET, ClientData> clients; // mapa atrybutów klientów
			int clientsCount = 0; // zmienna identyfikująca liczbę klientów

			bool connectWithClient(); // połączenie z klientem i dodanie go do wektora
			bool readHeader(const SOCKET& cSock, SHORT& cEv); // odczytywanie nagłówka
			bool sendFileToClient(const SOCKET& cSock); // wysyłanie pliku do klienta
			bool recvFileFromClient(const SOCKET& cSock); // odbieranie pliku od klienta
			std::vector<pollfd>::iterator disconnectWithClient(std::vector<pollfd>::iterator& it); // rozłączenie z klientem
			void logDetails(const std::string& type, const std::string& mess, // drukowanie informacji
				const SOCKET& cSock, const ClientData& cData, const std::string file, const int line) const;
			void closeConnection(); // zamknięcie socketu

		public:
			explicit Server(const Server&) = delete; // usunięcie konstruktora kopiującego
			explicit Server(Server&&) = delete; // usunięcie konstruktora przenoszącego
			explicit Server(); // kontruktor inicjalizujący serwer domyślnymi wartościami
			explicit Server(const char* address, const int& port); // konstruktor inicjalizujący serwer
			virtual ~Server(); // destruktor zwalniający zasoby

			Server& operator= (const Server&) = delete; // usunięcie operatora przypisania (kopiowanie)
			Server& operator= (Server&&) = delete; // usunięcie operatora przypisania (przenoszenie)

			bool initialize(); // inicjalizacja serwera
			void mainLoop(); // główna pętla serwera
			void closeAllConnections(); // zamknij wszystkie aktywne połączenia
	};
}

#endif // SERVER_H