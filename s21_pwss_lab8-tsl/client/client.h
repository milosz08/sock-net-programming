// program klienta - sygnatura klasy
#ifndef CLIENT_H
#define CLIENT_H

#include <lib\wsapi.h>
#include <lib\ssl_context.h>

namespace SOCK_NET
{
	// domyślna szerokość i długość geograficzna (Gliwice)
	const double DEF_LATITUDE = 50.33;
	const double DEF_LOGITUDE = 18.68;

	class Client : public SslContext
	{
		private:
			std::string readerRawData; // odczytany JSON w formie surowego ciągu znaków
			// długość i szerokość geograficzna
			double latitude = DEF_LATITUDE, logitude = DEF_LOGITUDE;

		public:
			Client(const Client&) = delete; // usunięcie konstruktora kopiującego
			Client(Client&&) = delete; // usunięcie konstruktora przenoszącego
			Client(); // konstruktor domyślny, inicjalizujący domyślnym adresem i portem
			Client(const std::string& address, const u_short& port); // konstruktor inicjalizujący klienta
			~Client(); // destruktor uruchamiany po zakończeniu pracy klienta

			Client& operator= (const Client&) = delete; // usunięcie operatora przypisania (kopiowanie)
			Client& operator= (Client&&) = delete; // usunięcie operatora przypisania (przenoszenie)

			bool connectWithServer(); // nawiązanie połączenia z serwerem
			bool sendDataToServer(); // wysłanie danych do serwera
			void insertLocationFromParamArgs(int& argc, char** argv);

		private:
			// odczytywanie danych z API pogodowego na podstawie przekazywanej długości i szerokości geog.
			bool readDataFromWeatherAPI();
			// wczytywanie danych lokalizacji z argumentów
			// zapisanie wyniku zwracanego z CURLa do ciągu znaków, źródło:
			// https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
			static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
	};
}

#endif // CLIENT_H