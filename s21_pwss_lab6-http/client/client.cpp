// program klienta - implementacja klasy
#include "client.h"

// konstruktor bezargumentowy wpisujący domyślne wartości adresu i portu do pola klasy
SOCK_NET::Client::Client() : address("127.0.0.1"), port(8080)
{
}

// konstruktor wpisujący wartości adresu i portu podane podczas tworzenia obiektu
SOCK_NET::Client::Client(const char* address, const int& port) : address(address), port(port)
{
}

SOCK_NET::Client::~Client()
{
	this->closeConnection(); // zamknij połączenie jeśli aktywne
}

void SOCK_NET::Client::closeConnection()
{
	if (this->sock == INVALID_SOCKET) return; // jeśli socket nie jest aktywny, wyjdź z metody
	closesocket(this->sock); // zamknij gniazdo
	this->sock = INVALID_SOCKET; // ustaw gniazdo na nieaktywne
	Lib::log(INFO_L, __FILE__, __LINE__,
		"Rozlaczenie z serwerem: " + std::string(this->address) + ":" + std::to_string(this->port) + ".");
}

bool SOCK_NET::Client::createConnection()
{
	struct sockaddr_in sa; // struktura do połączenia się z serwerem

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli i zwróci false
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane utworzenie socketu.");
		this->closeConnection(); // zamknięcie połączenia
		return false;
	}

	sa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	sa.sin_port = htons(this->port); // port serwera (pole klasy przypisywane konstruktorem)

	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów, jeśli błąd false
	if (inet_pton(AF_INET, this->address, &(sa.sin_addr)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane przypisanie IPv4 do struktury.");
		this->closeConnection(); // zamknięcie połączenia
		return false;
	}

	// parametry połączenia zapisane w postaci 0.0.0.0:0000
	std::string connDetails = std::string(this->address) + ":" + std::to_string(this->port);
	// zainicjalizuj połączenie z serwerem, jeśli nie uda się zwróci -1 i zakończy oraz zwróci false
	if (connect(this->sock, (sockaddr*)&sa, sizeof(sa)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane polaczenie z serwerem: " + connDetails + ".");
		this->closeConnection(); // zamknięcie połączenia
		return false;
	}
	Lib::log(INFO_L, __FILE__, __LINE__, "Nawiazano polaczenie z serwerem: " + connDetails + ".");
	return true; // jeśli wszystko ok, zwróć true
}

void SOCK_NET::Client::insertResourceName()
{
	std::string inputStream; // wprowadzany ciąg z klawiatury
	Lib::log(INFO_L, __FILE__, __LINE__, "Wprowadz nazwe zasobu do pobrania metoda GET:");
	while (true)
	{
		std::cout << ">> ";
		std::cin >> inputStream;
		if (inputStream == "") // jeśli wprowadzona nazwa zasobu jest pustym ciągiem znaków, kontynuuj
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nazwa zasobu nie moze byc pustym ciagiem znakow.");
			continue;
		}
		break;
	}
	this->resourceName = inputStream;
	Lib::log(INFO_L, __FILE__, __LINE__, "Dodanie nazwy zasobu: '" + this->resourceName 
		+ "' pobieranego poprzez metode GET.");
}

void SOCK_NET::Client::insertHeaderParams()
{
	int paramN = 0; // liczba parametrów zapytania
	Lib::log(INFO_L, __FILE__, __LINE__, "Podaj liczbe parametrow GET zapytania:");
	while (true)
	{
		std::cout << ">> ";
		std::cin >> paramN;
		if (!std::cin.good()) // jeśli wprowadzona wartość nie jest liczbą
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nieprawidlowa liczba parametrow zapytania GET.");
			std::cin.clear(); // czyszczyenie strumienia wejścia
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignorowanie znaków do nowej linii
			continue;
		}
		else if (paramN < 0) // jeśli wprowadzona liczba jest mniejsza od 0
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Ilosc parametrow GET zapytania musi byc nieujemna.");
			continue;
		}
		break;
	}
	for (int i = 0; i < paramN; i++)
	{
		std::string inputStream, key, value; // parametr oraz wydzielony z niego klucz oraz wartość
		Lib::log(INFO_L, __FILE__, __LINE__, "Wprowadz " + std::to_string(i + 1) + " parametr w konwencji 'KLUCZ=WARTOSC':");
		while (true)
		{
			std::cout << ">> ";
			std::cin >> inputStream;
			if (inputStream.find('=') == std::string::npos) // jeśli nie znajdzie delimitera parametrów, kontynuuj
			{
				Lib::log(ERROR_L , __FILE__, __LINE__, "Podane parametry nalezy rozdzielic znakiem '='.");
				continue;
			}
			key = inputStream.substr(0, inputStream.find('='));
			// drugą cześć po delimiterze przypisz do nazwy pliku
			value = inputStream.substr(inputStream.find('=') + 1, inputStream.size());
			if (key == "" || value == "") // jeśli wprowadzony klucz lub wartość jest pustym ciągiem znaków, kontynuuj
			{
				Lib::log(ERROR_L, __FILE__, __LINE__, "Parametr oraz wartosc parametru nie moze byc pustym ciagiem znakow.");
				continue;
			}
			break; // jeśli wszystko ok, przerwij pętlę
		}
		this->parameters.insert({ key, value });
		Lib::log(INFO_L , __FILE__, __LINE__, "Dodanie parametru '" + key + "=" + value + "' do listy parametrow GET.");
	}
}

bool SOCK_NET::Client::completeAndSendHeader()
{
	std::string params;
	if (this->parameters.size() != 0) // przejdź, jeśli podano jakieś parametry GET
	{
		params += "?"; // dodaj znak rozpoczynający ciąg parametrów
		int i = 0; // kolejny parametr
		for (auto& param : this->parameters)
		{
			if (i++ > 0) params += "&"; // dodaj znak konkatenacji dla wszytkich oprócz 1 parametru
			params += param.first + "=" + param.second; // konkatenacja iterowanego parametru z mapy
		}
	}
	
	std::string header = "GET " + this->resourceName + params + " HTTP/1.0\n\n"; // kompletowanie całego nagłówka

	size_t sendData = 0; // ilość wysłanych bajtów
	int sendRes = 0; // ilość bajtów zwróconych przez funckję send()
	size_t remainingData = header.size(); // ilość pozostałych danych do wysłania
	// wysyłaj dane z nagłówkiem do serwera, jeśli send zwróci tylko część danych dopisz do zmiennej sendData
	// i wysyłaj tak długo, aż pozostanie 0 danych do wysłania (zmienna remainingData)
	do
	{
		// wyślij dane z nagłówkiem do serwera, jeśli błąd wyświetl komunikat i zakończ działanie klienta
		if ((sendRes = send(this->sock, (char*)(header.c_str() + sendData), remainingData, 0)) < 0)
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane wyslanie naglowka do serwera.");
			return false;
		}
		else
		{
			sendData += sendRes; // dodaj ilość już wysłanych bajtów
			remainingData -= sendRes; // odejmij ilość pozostałych bajtów do wysłania
		}
	}
	while (remainingData > 0); // wysyłaj dane, dopóki pozostaną jakieś dane do wysłania
	Lib::log(INFO_L, __FILE__, __LINE__, "Wyslano do serwera naglowek: '" + header.substr(0, header.size() - 2) + "'.");
	return true;
}

void SOCK_NET::Client::receiveAndDecodeToken()
{
	int recvData = 0; // pobrane bajty przez funkcję recv()
	char buffer[FRAME_BUFF]; // bufor bajtów w rozmiarze ramki
	std::string recvContent; // cała treść zwrócona przez recv()

	while ((recvData = recv(this->sock, buffer, FRAME_BUFF, 0)) > 0)
	{
		// pomiń wszystkie niepożądane znaki z bufora przy zapisywaniu do końcowego rezultatu
		for (int i = 0; i < recvData; i++) if (buffer[i] != '\0') recvContent += buffer[i];
	}
	if (recvData < 0) // jeśli funkcja recv() zwróci -1
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane odebranie ramki danych z serwera.");
		this->closeConnection();
		return;
	}

	size_t tokenPos = recvContent.find("TOKEN"); // znajdź pozycję "TOKEN" w ciągu wejściowym
	// jeśli nie znajdzie zakończ działanie i rozłącz z serwerem
	if (tokenPos == std::string::npos)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Ciag 'TOKEN' nie zostal odnaleziony w podanym tekscie.");
	}
	else // w przeciwnym wypadku wyodrębnij i dekoduj token
	{
		// Wyodrębnij wartość tokena, znajdując pozycję następnego znaku nowej linii
		// i przejęcie podłańcucha z pozycji po "TOKEN" na znak nowej linii
		size_t newlinePos = recvContent.find('\n', tokenPos);
		std::string exprContent = recvContent.substr(tokenPos + 10, newlinePos - tokenPos - 10);
		std::string tokenEnc = exprContent.substr(0, exprContent.find("<br>"));

		std::vector<u_char> decodedBytes = Lib::base64decode(tokenEnc); // odkodowanie tokenu
		std::string tokenDec(decodedBytes.begin(), decodedBytes.end()); // konwersja wektora bajtów na string

		Lib::log(INFO_L, __FILE__, __LINE__, "Odbieranie danych z serwera zakonczone.");
		Lib::log(INFO_L, __FILE__, __LINE__, "Token zakodowany w Base64:\n\n" + tokenEnc + "\n");
		Lib::log(INFO_L, __FILE__, __LINE__, "Token odkodowany:\n\n" + tokenDec + "\n");
	}
	this->closeConnection(); // zamknięcie połączenia
}