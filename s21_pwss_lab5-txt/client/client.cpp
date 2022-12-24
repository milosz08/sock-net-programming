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

bool SOCK_NET::Client::createConnection()
{
	struct sockaddr_in sa; // struktura do połączenia się z serwerem

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli i zwróci false
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane utworzenie socketu.");
		return false;
	}

	sa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	sa.sin_port = htons(this->port); // port serwera (pole klasy przypisywane konstruktorem)

	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów, jeśli błąd false
	if (inet_pton(AF_INET, this->address, &(sa.sin_addr)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane przypisanie IPv4 do struktury.");
		return false;
	}

	std::string connInfo = std::string(this->address) + ":" + std::to_string(this->port);
	// zainicjalizuj połączenie z serwerem, jeśli nie uda się zwróci -1 i zakończy oraz zwróci false
	if (connect(this->sock, (sockaddr*)&sa, sizeof(sa)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane polaczenie z serwerem: " + connInfo + ".");
		return false;
	}
	Lib::log(INFO_L, __FILE__, __LINE__, "Nawiazano polaczenie z serwerem: " + connInfo + ".");
	return true; // jeśli wszystko ok, zwróć true
}

void SOCK_NET::Client::selectAction()
{
	std::string inputStream, action, fileName, mode; // ciąg wejściowy od użytkownika, akcja i nazwa pliku i tryb
	Lib::log(INFO_L, __FILE__, __LINE__, "Wprowadz typ akcji w formacie '(SEND|RECV)" + std::to_string(DELIMITER) 
		+ "FILENAME', gdzie FILENAME to nazwa pliku:");
	while (true)
	{
		std::cout << ">> ";
		std::cin >> inputStream; // wczytaj ciąg wejściowy
		// jeśli nie znajdzie delimitera, wyświetl błąd i ponów wpisanie akcji przez użytkownika
		if (inputStream.find(DELIMITER) == std::string::npos)
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nalezy podac dwa parametry rozdzielone delimiterem.");
			continue;
		}
		// rozdziel ciąg wejściowy na podstawie delimitera na akcję
		action = inputStream.substr(0, inputStream.find(DELIMITER));
		// drugą cześć po delimiterze przypisz do nazwy pliku
		fileName = inputStream.substr(inputStream.find(DELIMITER) + 1, inputStream.size());
		// sprawdź, czy parametr action jest inny niż SEND bądź RECV, jeśli tak error i przejdź na
		// początek pętli w celu ponownego wprowadzenia danych
		if (_strcmpi(action.c_str(), "SEND") == 0)
		{
			this->action = SOCK_NET::Action::SEND; // ustaw akcję na wysyłanie pliku do serwera
			mode = "rb"; // ustaw tryb otwarcia pliku na readbytes
		}
		else if (_strcmpi(action.c_str(), "RECV") == 0)
		{
			this->action = SOCK_NET::Action::RECV; // ustaw akcję na odbieranie pliku z serwera
			mode = "wb"; // ustaw tryb otwarcia pliku na writebytes
		}
		else
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Prawidlowe parametry akcji to 'SEND' lub 'RECV'.");
			continue;
		}
		if (fileName == "") // sprawdź, czy podawana nazwa nie jest pustym ciągiem znaków
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nazwa pliku nie moze byc pustym ciagiem znakow.");
			continue;
		}
		this->fileHandler = fopen(fileName.c_str(), mode.c_str()); // handler do pliku
		// jeśli nie znajdzie pliku na podstawie wybranej nazwy lub inny bład przejdź do początku pętli
		if (this->fileHandler == nullptr)
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane otwarcie pliku '" + fileName + "'.");
			continue;
		}
		break; // jeśli wszystko ok, wyjdź z pętli
	}
	this->fileName = fileName; // przypisz nazwę pliku do pól klasy
	this->inputStream = inputStream + SEND_INDC; // dodaj sekwencję kończącą
}

bool SOCK_NET::Client::sendHeader()
{
	size_t sendData = 0; // ilość wysłanych bajtów
	int sendRes = 0; // ilość bajtów zwróconych przez funckję send()
	size_t remainingData = this->inputStream.size(); // ilość pozostałych danych do wysłania
	// wysyłaj dane z nagłówkiem do serwera, jeśli send zwróci tylko część danych dopisz do zmiennej sendData
	// i wysyłaj tak długo, aż pozostanie 0 danych do wysłania (zmienna remainingData)
	do
	{
		// wyślij dane z nagłówkiem do serwera, jeśli błąd wyświetl komunikat i zakończ działanie klienta
		if ((sendRes = send(this->sock, (char*)(this->inputStream.c_str() + sendData), remainingData, 0)) < 0)
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
	while (remainingData > 0); // wysyłaj dane, dopóki send będzie zwracał wartości większe od 0
	// usuń z nagłówka znak informujący o jego końcu
	const std::string withoutEOL = this->inputStream.substr(0, this->inputStream.size() - 1);
	Lib::log(INFO_L , __FILE__, __LINE__, "Wyslano do serwera naglowek: '" + withoutEOL + "'.");
	return true; // wszystko ok, zwróć true
}

void SOCK_NET::Client::sendFile()
{
	char buffer[FRAME_BUFF]; // bufor na wysyłane dane
	fseek(this->fileHandler, 0, SEEK_END); // ustaw pozycję kursora na końcu pliku
	int fileFullSize = ftell(this->fileHandler); // pobierz ilość bajtów pliku
	int frames = 0; // ilość wysłanych ramek
	bool error = false; // błąd podczas wysyłania danych

	// pętla iterująca do momentu, kiedy prześle wszystkie bajty (ilość ramek * jej rozmiar) oraz jeśli
	// zmienna error jest równa false
	while (((frames * FRAME_BUFF) < fileFullSize) && !error)
	{
		// przesuń pozycję kursora na podstawie pobieranej ramki (ilość pobranych ramek * jej rozmiar),
		fseek(this->fileHandler, (frames * FRAME_BUFF), SEEK_SET);
		// odczytaj jedną ramkę pliku i zwróć faktycznie odczytaną ilość bajtów
		size_t singleFrameFileSize = fread(buffer, 1, FRAME_BUFF, this->fileHandler);
		size_t sendData = 0; // ilość wysłanych bajtów
		int sendRes = 0; // ilość wysłanych danych przez send()
		size_t remainingData = singleFrameFileSize; // ilość pozostałych danych do wysłania
		do
		{
			// wyślij dane z nagłówkiem do serwera, jeśli błąd wyświetl komunikat i zakończ działanie klienta
			if ((sendRes = send(this->sock, (char*)(buffer + sendData), remainingData, 0)) < 0)
			{
				error = true; // ustaw error na true
				break; // wyjdź z pętli
			}
			else
			{
				sendData += sendRes; // dodaj ilość już wysłanych bajtów
				remainingData -= sendRes; // odejmij ilość pozostałych bajtów do wysłania
			}
		}
		while (remainingData > 0); // wysyłaj dane, dopóki send będzie zwracał wartości większe od 0
		// wyświetl procent wysłania danych przez klienta
		float percentage = ((++frames * FRAME_BUFF) / (float)fileFullSize) * 100;
		std::cout << "\r";
		Lib::log(INFO_L, __FILE__, __LINE__, "Wysylanie w toku... " + std::to_string((int)percentage) + "%", false);
		std::cout << std::flush;
	}
	// dodawaj nową linię tylko wtedy, gdy nie ma błędu i gdy pobrano jakieś bajty
	if (!error || fileFullSize > 0) std::cout << "\n";
	// jeśli wystąpił error w wysyłaniu danych (np. utracone połączenie z serwerem)
	if (error) Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane wyslanie ramki danych do serwera.");
	else
	{
		Lib::log(INFO_L , __FILE__, __LINE__, "Wyslano " + std::to_string(frames) + " ramek, rozmiar " 
			+ std::to_string(FRAME_BUFF) + " bajtow.");
		Lib::log(INFO_L, __FILE__, __LINE__, "Calkowity rozmiar pliku: " + std::to_string(fileFullSize) + " bajtow.");
	}
	fclose(this->fileHandler); // zamknięcie pliku
	this->closeConnection(); // przy wysłaniu wszystkich danych lub jeśli błąd zamknij połączenie
}

void SOCK_NET::Client::receiveFile()
{
	size_t allRecvData = 0; // ilość odebranych danych przez recv
	int recvData = 0; // pobrane bajty przez funkcję recv()
	char buffer[FRAME_BUFF]; // bufor bajtów w rozmiarze ramki
	
	// odczytuj do momentu, gdy recv zwraca bajty
	while ((recvData = recv(this->sock, buffer, FRAME_BUFF, 0)) > 0)
	{
		fwrite(buffer, sizeof(char), recvData, this->fileHandler); // dopisz zawartość bufora do pliku
		fflush(this->fileHandler); // odśwież status pliku
		allRecvData += recvData; // zsumowana ilość pobranych bajtów pliku z serwera

		std::cout << "\r";
		Lib::log(INFO_L, __FILE__, __LINE__, "Pobieranie w toku... " + std::to_string(allRecvData) + "bajtow.", false);
		std::cout << std::flush;
	}
	if (allRecvData > 0) std::cout << "\n"; // dodawaj nową linię tylko wtedy, gdy odbierze jakieś bajty
	// jeśli nie uda się odczytać ramki zakończ połączenie z serwerem
	if (recvData < 0) Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane odebranie ramki danych z serwera.");
	else
	{
		Lib::log(INFO_L, __FILE__, __LINE__, "Odbieranie danych z serwera zakonczone. Pobrano " + std::to_string(allRecvData)
				 + " bajtow. Rozmiar ramki: " + std::to_string(FRAME_BUFF) + " bajtow.");
	}
	this->closeConnection(); // przy odebraniu wszystkich danych lub jeśli błąd zamknij połączenie
	fclose(this->fileHandler); // zamknięcie pliku
}

SOCK_NET::Action SOCK_NET::Client::getAction() const
{
	return this->action; // zwróć aktualną akcję
}

void SOCK_NET::Client::closeConnection()
{
	if (this->sock == INVALID_SOCKET) return; // jeśli gniazdo nieaktywne, zakończ działanie metody
	closesocket(this->sock); // zamknij gniazdo
	this->sock = INVALID_SOCKET; // ustaw gniazdo na nieaktywne
	Lib::log(INFO_L, __FILE__, __LINE__, "Rozlaczenie z serwerem " + std::string(this->address) + ":"
		+ std::to_string(this->port) + ".");
}