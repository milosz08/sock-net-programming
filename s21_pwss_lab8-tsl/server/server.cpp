#include "server.h"

// konstruktor bezargumentowy wpisujący domyślne wartości adresu i portu do pola klasy
SOCK_NET::Server::Server() : SslContext(DEF_HOST, DEF_PORT)
{
	WsAPI::logInfo("Inicjalizacja obiektu serwera domyslnymi wartosciami", __FILE__, __LINE__);
}

// konstruktor wpisujący wartości adresu i portu podane podczas tworzenia obiektu
SOCK_NET::Server::Server(const std::string& address, const u_short& port) : SslContext(address, port)
{
	WsAPI::logInfo("Inicjalizacja obiektu serwera z adresem: " + address + ":" + std::to_string(port), __FILE__, __LINE__);
}

SOCK_NET::Server::~Server()
{
	WsAPI::logInfo("Konczenie dzialania serwera", __FILE__, __LINE__);
}

bool SOCK_NET::Server::initializeServer()
{
	struct sockaddr_in sa; // struktura serwera

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli i zwróci false
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		WsAPI::logError("Nieudane utworzenie socketu", __FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Pomyslne utworzenie socketu", __FILE__, __LINE__);

	sa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	sa.sin_port = htons(this->port); // port serwera

	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów
	if (inet_pton(AF_INET, this->address.c_str(), &(sa.sin_addr)) < 0)
	{
		WsAPI::logError("Nieudane przypisanie IPv4 do struktury", __FILE__, __LINE__);
		return false;
	}

	// przypisanie struktury do socketu, jeśli zwróci -1 błąd i zwrócenie false
	if (bind(this->sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
	{
		WsAPI::logError("Nieudane powiazanie socketu ze struktura", __FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Powiazanie socketu ze struktura sockaddr_in", __FILE__, __LINE__);

	// przełączenie socketu w tryb nasłuchiwania połączeń przychodzących poprzez
	// funkcję accept(), jeśli błąd zwróci -1 i zwróci wartość false
	if (listen(this->sock, SOMAXCONN) < 0)
	{
		WsAPI::logError("Nieudane uruchomienie serwera w trybie nasluchu", __FILE__, __LINE__);
		return false;
	}
	WsAPI::logInfo("Serwer z IP " + std::string(this->address) + " slucha na porcie "
		+ std::to_string(this->port) + "...", __FILE__, __LINE__);
	return true; // jeśli wszystko ok, zwróć true i uruchom główną pętlę serwera
}

void SOCK_NET::Server::receiveData()
{
	SOCKET cSock = INVALID_SOCKET; // socket nowego klienta
	struct sockaddr_in cSa; // struktura klienta wypełniania przez funkcję accept()
	int cSaSize = sizeof(cSa); // wielkość struktury klienta
	this->ssl = nullptr; // ustaw na pusty wskaźnik, przy kolejnym obsługiwaniu klienta

	char clientIp[INET_ADDRSTRLEN]; // adres IP klienta
	// akceptuj połączenie klienta, jeśli napotka błąd zwróci -1 i nie doda do wektora klientów
	if ((cSock = accept(this->sock, (sockaddr*)&cSa, &cSaSize)) < 0)
	{
		WsAPI::logError("Nieudane polaczenie z klientem", __FILE__, __LINE__);
		closesocket(cSock); // zamknij socket klienta
		return; // przejdź na początek pętli w celu obsłużenia nowego klienta
	}
	// konwersja adresu IP w formie bajtów na ciąg znaków, jeśli nie uda się zwróci NULL
	if ((inet_ntop(AF_INET, &(((struct sockaddr_in*)&cSa)->sin_addr), clientIp, sizeof(clientIp)) == NULL))
	{
		WsAPI::logError("Nieudane odczytanie adresu IP", __FILE__, __LINE__);
		disconnectWithClient(clientIp, cSock);
		return; // przejdź na początek pętli w celu obsłużenia nowego połączenia
	}

	this->ssl = SSL_new(this->sslContext); // utwórz nowy obiekt SSL i przypisz do pola
	if (this->ssl == nullptr) // jeśli nie udało się utworzyć, rozłącz z klientem
	{
		WsAPI::logError(std::string(clientIp) + " <> Nieudane stworzenie obiektu SSL", __FILE__, __LINE__);
		disconnectWithClient(clientIp, cSock);
		return; // przejdź na początek pętli w celu obsłużenia nowego połączenia
	}

	// nawiązanie połączenia z klientem poprzez TLS, jeśli nie powiedzie się powiązać gniazda z obiektem
	// lub nie uda się zaakceptować połączenia, rozłącz z klientem
	if (SSL_set_fd(this->ssl, cSock) != 1 || SSL_accept(this->ssl) != 1)
	{
		WsAPI::logError(std::string(clientIp) + " <> Nieudane stworzenie bezpiecznego polaczenia", __FILE__, __LINE__);
		SSL_free(this->ssl);
		disconnectWithClient(clientIp, cSock);
		return; // przejdź na początek pętli w celu obsłużenia nowego połączenia
	}
	WsAPI::logInfo(std::string(clientIp) + " <> Nawiazano bezpieczne polaczenie z klientem", __FILE__, __LINE__);

	char buffer[FRAME_BUFF_SIZE]; // bufor na dane wysyłane przez klienta
	int revcSize;
	std::string weatherResultStringData; // ciąg wynikowy z danymi pogodowymi

	// odczytuj dane do momentu zwrócenia wartości mniejszej od 1
	while ((revcSize = SSL_read(this->ssl, buffer, FRAME_BUFF_SIZE)) > 0)
	{
		// przejdź kolejno przez wszystkie odczytane dane i dopisz do ciągu wynikowego
		for (int i = 0; i < revcSize; i++) weatherResultStringData += buffer[i];
	}
	this->decodeJsonDataAndPrint(weatherResultStringData);
	this->closeSslConnection(); // zamknij połączenie SSL
	this->disconnectWithClient(clientIp, sock); // zamknij socket klienta
}

void SOCK_NET::Server::decodeJsonDataAndPrint(std::string& data)
{
	Json::Reader reader; // obiekt biblioteki JSON
	Json::Value js; // odczytywana wartość w formacie JSON
	// wartości domyślne temperatury oraz lokalizacji
	double latitude = 0.00, logitude = 0.00, temperature = 0.0;
	//double latitude = 0.00, logitude = 0.00, temperature = 0.0;
	if (!reader.parse(data, js)) // parsuj JSON
	{
		WsAPI::logError("Wystapil problem z parsowaniem danych JSON", __FILE__, __LINE__);
		return;
	}
	// sprawdź, czy JSON zawiera obiekt current_weather, jeśli nie zakończ działanie
	if (!js.isObject() || !js.isMember("current_weather"))
	{
		WsAPI::logError("JSON nie zawiera obiektu 'current_weather'", __FILE__, __LINE__);
		return;
	}
	Json::Value cr = js["current_weather"]; // pobierz obiekt current_weather
	if (js["latitude"].isDouble())		latitude = js["latitude"].asDouble();
	if (js["logitude"].isDouble())		logitude = js["logitude"].asDouble();
	if (cr["temperature"].isDouble())	temperature = cr["temperature"].asDouble();

	std::cout << '\n';
	std::cout << "\tOtrzymane dane pogodowe:\n";
	std::cout << "\tTemperatura:" << std::setprecision(1) << temperature << "\370C" << '\n';
	std::cout << "\tSzerokosc georagraficzna:" << std::setprecision(2) << latitude << '\n';
	std::cout << "\tDlugosc geograficzna:" << std::setprecision(2) << logitude << '\n';
	std::cout << '\n';
}

void SOCK_NET::Server::disconnectWithClient(char* ip, SOCKET& sock)
{
	WsAPI::logInfo(std::string(ip) + " <> Rozlaczenie z klientem", __FILE__, __LINE__);
	closesocket(sock); // zamknij socket klienta
}
