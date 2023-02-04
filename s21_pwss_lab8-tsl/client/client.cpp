#include "client.h"

// konstruktor bezargumentowy wpisujący domyślne wartości adresu i portu do pola klasy
SOCK_NET::Client::Client() : SslContext(DEF_HOST, DEF_PORT)
{
	WsAPI::logInfo("Inicjalizacja obiektu klienta domyslnymi wartosciami", __FILE__, __LINE__);
}

// konstruktor wpisujący wartości adresu i portu podane podczas tworzenia obiektu
SOCK_NET::Client::Client(const std::string& address, const u_short& port) : SslContext(address, port)
{
	WsAPI::logInfo("Inicjalizacja obiektu klienta", __FILE__, __LINE__);
}

SOCK_NET::Client::~Client()
{
	WsAPI::logInfo("Konczenie dzialania klienta", __FILE__, __LINE__);
}

bool SOCK_NET::Client::connectWithServer()
{
	struct sockaddr_in sa; // struktura do połączenia się z serwerem

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli i zwróci false
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		WsAPI::logError("Nieudane utworzenie socketu", __FILE__, __LINE__);
		return false;
	}

	sa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	sa.sin_port = htons(this->port); // port serwera (pole klasy przypisywane konstruktorem)

	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów, jeśli błąd false
	if (inet_pton(AF_INET, this->address.c_str(), &(sa.sin_addr)) < 0)
	{
		WsAPI::logError("Nieudane przypisanie IPv4 do struktury", __FILE__, __LINE__);
		return false;
	}

	std::string connInfo = this->address + ":" + std::to_string(this->port);
	// zainicjalizuj połączenie z serwerem, jeśli nie uda się zwróci -1 i zakończy oraz zwróci false
	if (connect(this->sock, (sockaddr*)&sa, sizeof(sa)) < 0)
	{
		WsAPI::logError("Nieudane polaczenie z serwerem: " + connInfo, __FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Nawiazano polaczenie z serwerem: " + connInfo, __FILE__, __LINE__);

	this->ssl = SSL_new(this->sslContext);
	if (this->ssl == nullptr)
	{
		WsAPI::logError("Nieudane utworzenie obiektu SSL", __FILE__, __LINE__);
		return false;
	}
	// nawiązanie połączenia z serwerem poprzez TLS, jeśli nie powiedzie się powiązać gniazda z obiektem
	// lub nie uda się zaakceptować połączenia, zwróć false
	if (SSL_set_fd(this->ssl, this->sock) != 1 || SSL_connect(this->ssl) != 1)
	{
		WsAPI::logError("Nieudane utworzenie bezpiecznego polaczenia z serwerem: " +
			connInfo, __FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Nawiazano bezpieczne polaczenie z serwerem: " + connInfo, __FILE__, __LINE__);
	return true; // jeśli wszystko ok, zwróć true
}

bool SOCK_NET::Client::readDataFromWeatherAPI()
{
	CURL* curl = curl_easy_init(); // inicjalizacja biblioteki CURL
	if (!curl) // w przypadku nieudanego zainicjalizowania, wypisz komunikat i zwróć false
	{
		WsAPI::logError("Nieudane zainicjalizowanie biblioteki CURL.", __FILE__, __LINE__);
		return false;
	}
	// adres na który CURL dokonuje zapytania
	std::string query = "https://api.open-meteo.com/v1/forecast"
		"?latitude=" + std::to_string(this->latitude) +
		"&longitude=" + std::to_string(this->logitude) +
		"&current_weather=true";

	curl_easy_setopt(curl, CURLOPT_URL, query.c_str()); // dodaj adres do wykonywanego zapytania
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->writeCallback); // wywołanie funkcji
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->readerRawData); // wpisz otrzymane dane do bufora
	
	CURLcode res = curl_easy_perform(curl); // wykonaj zapytanie
	if (res != CURLE_OK) // jeśli nie powiodło się, wyświetl błąd i zakończ działanie
	{
		WsAPI::logError("Nieudane wykonanie zapytania Error: " + 
			std::string(curl_easy_strerror(res)),__FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Wykonanie zapytania do API i wpisanie danych do bufora.", __FILE__, __LINE__);
	curl_easy_cleanup(curl); // zwalnianie zasobów
	return true;
}

void SOCK_NET::Client::insertLocationFromParamArgs(int& argc, char** argv)
{
	bool foundLatitude = false; // flaga informująca o pobraniu szerokości geogr.
	bool foundLongitude = false; // flaga informująca o pobraniu długości geogr.
	std::string latitude, longitude; // zapisane długość i szerokość geograficzna
	for (int i = 0; i < argc; i++) // przejdź przez wszystkie podane argumenty
	{
		std::string param = argv[i]; // przypisz do stringa aby móc używać jego metod
		int separatorLocation = param.find('='); // znajdź separator w argumencie
		if (separatorLocation < 0) continue; // jeśli nie znajdzie, przejdź kolejną iterację

		std::string key = param.substr(0, separatorLocation); // klucz
		std::string value = param.substr(separatorLocation + 1, param.size()); // wartość

		if (key == "--latitude") // znajdź paramater przechowujący szerokość geogr.
		{
			latitude = value; // zapisz szerokość do zmiennej
			foundLatitude = true; // ustaw flagę na znaleziony
		}
		else if (key == "--longitude") // znajdź paramater przechowujący długość geogr.
		{
			longitude = value; // zapisz długość do zmiennej
			foundLongitude = true; // ustaw falgę na znaleziony
		}
	}
	// konwertuj na double i sprawdź, czy mieści się w przedziale
	if (WsAPI::checkIfValueIsDouble(-90.00, 90.00, latitude, &this->latitude))
	{
		WsAPI::logInfo("Zainicjalizowano szerokosc georgraficzna: " +
			std::to_string(this->latitude), __FILE__, __LINE__);
	}
	// jeśli nie przypisz wartość domyślną
	else WsAPI::logInfo("Zainicjalizowano domyslna szerokosc georgraficzna: " +
		std::to_string(DEF_LATITUDE), __FILE__, __LINE__);
	
	// konwertuj na double i sprawdź, czy mieści się w przedziale
	if (WsAPI::checkIfValueIsDouble(-180.00, 180.00, longitude, &this->logitude))
	{
		WsAPI::logInfo("Zainicjalizowano dlugosc georgraficzna: " +
			std::to_string(this->logitude), __FILE__, __LINE__);
	}
	// jeśli nie przypisz wartość domyślną
	else WsAPI::logInfo("Zainicjalizowano domyslna dlugosc georgraficzna: " +
		std::to_string(DEF_LOGITUDE), __FILE__, __LINE__);
}

size_t SOCK_NET::Client::writeCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool SOCK_NET::Client::sendDataToServer()
{
	if(!this->readDataFromWeatherAPI()) return false; // pobierz dane z API, jeśli błąd zakończ

	int sendRes; // ilość wysłanych bajtów przez SSL_write
	size_t payloadSize = this->readerRawData.size(); // rozmiar wysyłanych danych
	// wyślij dane do serwera, jeśli błąd wyświetl komunikat i rozłącz z serwerem
	if ((sendRes = SSL_write(this->ssl, this->readerRawData.c_str(), payloadSize)) < 0)
	{
		WsAPI::logError("Nieudane wyslanie ramki danych do serwera", __FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Pomyslne wyslanie " + std::to_string(sendRes) + " bajtow danych do serwera",
		__FILE__, __LINE__);
	return true;
}
