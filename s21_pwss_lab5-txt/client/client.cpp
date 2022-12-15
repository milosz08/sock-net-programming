// program klienta - implementacja klasy
#include "client.h"

// konstruktor bezargumentowy wpisujący domyślne wartości adresu i portu do pola klasy
SOCK_NET::Client::Client() : address("127.0.0.1"), port(8080)
{
}

// konstruktor wpisujący wartości adresu i portu podane podczas instancjacji obiektu
SOCK_NET::Client::Client(const char* address, const int& port) : address(address), port(port)
{
}

SOCK_NET::Client::~Client()
{
	if (this->sock != INVALID_SOCKET) this->closeConnection(); // zamknij połączenie jeśli aktywne
}

bool SOCK_NET::Client::createConnection()
{
	struct sockaddr_in sa; // struktura do połączenia się z serwerem

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli i zwróci false
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		std::cerr << "[ERROR::CLIENT]\tNieudane utworzenie socketu.\n";
		this->closeConnection(); // zamknij socket
		return false;
	}
	std::cout << "[INFO::CLIENT]\tPomyslne utworzenie socketu.\n";

	sa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	sa.sin_port = htons(this->port); // port serwera (pole klasy przypisywane konstruktorem)

	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów, jeśli błąd false
	if (inet_pton(AF_INET, this->address, &(sa.sin_addr)) < 0)
	{
		std::cerr << "[ERROR::CLIENT]\tNieudane przypisanie IPv4 do struktury.\n";
		this->closeConnection(); // zamknij socket
		return false;
	}

	// zainicjalizuj połączenie z serwerem, jeśli nie uda się zwróci -1 i zakończy zwróci false
	if (connect(this->sock, (sockaddr*)&sa, sizeof(sa)) < 0)
	{
		std::cerr << "[ERROR::CLIENT]\tNieudane polaczenie z serwerem: ";
		std::cerr << this->address << ":" << this->port << ".\n";
		this->closeConnection(); // zamknij socket
		return false;
	}
	std::cout << "[INFO::CLIENT]\tNawiazano polaczenie z serwerem: ";
	std::cout << this->address << ":" << this->port << ".\n";
	return true;
}

void SOCK_NET::Client::selectAction()
{
	std::string inputStream, action, fileName; // ciąg wejściowy od użytkownika, akcja i nazwa pliku
	std::cout << "[INFO::CLIENT]\tWprowadz typ akcji w formacie '(SEND|RECV)" << SOCK_NET::Lib::DELIMITER;
	std::cout << "FILENAME', gdzie FILENAME to nazwa pliku:\n";
	while (true)
	{
		std::cout << ">> ";
		std::cin >> inputStream; // wczytaj ciąg wejściowy
		// jeśli nie znajdzie delimitera, wyświetl błąd i ponów wpisanie akcji przez użytkownika
		if (inputStream.find(SOCK_NET::Lib::DELIMITER) == std::string::npos)
		{
			std::cerr << "[ERROR::CLIENT]\tNalezy podac dwa parametry rozdzielone delimiterem.\n";
			continue;
		}
		// rozdziel ciąg wejściowy na podstawie delimitera na akcję
		action = inputStream.substr(0, inputStream.find(SOCK_NET::Lib::DELIMITER));
		// drugą cześć po delimiterze przypisz do nazwy pliku
		fileName = inputStream.substr(inputStream.find(SOCK_NET::Lib::DELIMITER) + 1, inputStream.size());
		// sprawdź, czy parametr action jest inny niż SEND bądź RECV, jeśli tak error i przejdź na
		// początek pętli w celu ponownego wprowadzenia danych
		if (_strcmpi(action.c_str(), "SEND") == 0) this->action = SOCK_NET::Action::SEND;
		else if (_strcmpi(action.c_str(), "RECV") == 0) this->action = SOCK_NET::Action::RECV;
		else
		{
			std::cerr << "[ERROR::CLIENT]\tPrawidlowe parametry akcji to 'SEND' lub 'RECV'.\n";
			continue;
		}
		// sprawdź, czy podawana nazwa nie jest pustym ciągiem znaków
		if (fileName == "")
		{
			std::cerr << "[ERROR::CLIENT]\tNazwa pliku nie moze byc pustym ciagiem znakow.\n";
			continue;
		}
		this->fileName = fileName; // przypisz nazwę pliku do pól klasy
		this->inputStream = inputStream + SOCK_NET::Lib::SEND_INDC; // dodaj sekwencję kończącą
		break; // jeśli wszystko ok, wyjdź z pętli
	}
}

bool SOCK_NET::Client::sendHeader()
{
	size_t sendData = 0, sendRes = 0; // ilość wysłanych bajtów
	size_t remainingData = this->inputStream.size(); // ilość pozostałych danych do wysłania
	// wysyłaj dane z nagłówkiem do serwera, jeśli send zwróci tylko część danych dopisz do zmiennej sendData
	// i wysyłaj tak długo, aż pozostanie 0 danych do wysłania (zmienna remainingData)
	do
	{
		// wyślij dane z nagłówkiem do serwera, jeśli błąd wyświetl komunikat i zakończ działanie klienta
		if ((sendRes = send(this->sock, (char*)(this->inputStream.c_str() + sendData), remainingData, 0)) < 0)
		{
			std::cerr << "[ERROR::CLIENT]\tNieudane wyslanie naglowka do serwera.\n";
			return false;
		}
		sendData += sendRes; // dodaj ilość już wysłanych bajtów
		remainingData -= sendRes; // odejmij ilość pozostałych bajtów do wysłania
	} while (remainingData > 0); // wysyłaj dane, dopóki send będzie zwracał wartości większe od 0
	// usuń z nagłówka znak informujący o jego końcu
	const std::string withoutEOL = this->inputStream.substr(0, this->inputStream.size() - 1);
	std::cout << "[INFO::CLIENT]\tWyslano do serwera naglowek: '" << withoutEOL << "'.\n";
	return true; // wszystko ok, zwróć true
}

void SOCK_NET::Client::sendFile()
{
	// odczytaj plik na podstawie nazwy, jeśli się nie powiedzie, wyświetl błąd i zakończ działanie klienta
	FILE* fileHandler = fopen(this->fileName.c_str(), "rb");
	if (fileHandler == nullptr)
	{
		std::cerr << "[ERROR::CLIENT]\tNieudane otwarcie pliku '" << this->fileName << "'.\n";
		return;
	}

	char buffer[SOCK_NET::Lib::FRAME_BUFF]; // bufor na wysyłane dane
	fseek(fileHandler, 0, SEEK_END); // ustaw pozycję kursora na końcu pliku
	int fileFullSize = ftell(fileHandler); // pobierz ilość bajtów pliku
	int frames = 0; // ilość wysłanych ramek

	// pętla iterująca do momentu, kiedy prześle wszystkie bajty (ilość ramek * jej rozmiar)
	while ((frames * SOCK_NET::Lib::FRAME_BUFF) < fileFullSize)
	{
		// przesuń pozycję kursora na podstawie pobieranej ramki (ilość pobranych ramek * jej rozmiar),
		fseek(fileHandler, (frames * SOCK_NET::Lib::FRAME_BUFF), SEEK_SET);
		// odczytaj jedną ramkę pliku i zwróć faktycznie odczytaną ilość bajtów
		size_t singleFrameFileSize = fread(buffer, 1, SOCK_NET::Lib::FRAME_BUFF, fileHandler);
		size_t sendData = 0, sendRes = 0; // ilość wysłanych bajtów
		size_t remainingData = singleFrameFileSize; // ilość pozostałych danych do wysłania
		do
		{
			// wyślij dane z nagłówkiem do serwera, jeśli błąd wyświetl komunikat i zakończ działanie klienta
			if ((sendRes = send(this->sock, (char*)(buffer + sendData), remainingData, 0)) < 0)
			{
				std::cerr << "[ERROR::CLIENT]\tNieudane wyslanie ramki danych do serwera.\n";
				return;
			}
			sendData += sendRes; // dodaj ilość już wysłanych bajtów
			remainingData -= sendRes; // odejmij ilość pozostałych bajtów do wysłania
		} while (remainingData > 0); // wysyłaj dane, dopóki send będzie zwracał wartości większe od 0
		// wyświetl procent wysłania danych przez klienta
		float percentage = ((++frames * SOCK_NET::Lib::FRAME_BUFF) / (float)fileFullSize) * 100;
		std::cout << "\r[INFO::CLIENT]\tWysylanie w toku... " << (int)percentage << "%" << std::flush;
	}
	std::cout << "\n";
	this->closeConnection(); // rozłączenie z serwerem
	std::cout << "[INFO::CLIENT]\tWyslano " << frames << " ramek, rozmiar " << SOCK_NET::Lib::FRAME_BUFF << " bajtow.\n";
	std::cout << "[INFO::CLIENT]\tCalkowity rozmiar pliku: " << fileFullSize << " bajtow.\n";
	fclose(fileHandler); // zamknięcie pliku
}

void SOCK_NET::Client::receiveFile()
{
	// utwórz plik na podstawie nazwy, jeśli się nie powiedzie, wyświetl błąd i zakończ działanie klienta
	FILE* fileHandler = fopen(this->fileName.c_str(), "wb");
	if (fileHandler == nullptr)
	{
		std::cerr << "[ERROR::CLIENT]\tNieudane utworzenie pliku '" << this->fileName << "'.\n";
		return;
	}
	size_t recvData = 0, allRecvData = 0; // ilość odebranych danych przez recv
	char buffer[SOCK_NET::Lib::FRAME_BUFF]; // bufor bajtów w rozmiarze ramki
	while ((recvData = recv(this->sock, buffer, SOCK_NET::Lib::FRAME_BUFF, 0)) > 0)
	{
		// jeśli nie uda się odczytać ramki, zakończ połączenie z serwerem
		if (recvData < 0)
		{
			std::cout << "\n[ERROR::CLIENT]\tNieudane odczytanie ramki danych.\n";
			return;
		}
		fwrite(buffer, sizeof(char), recvData, fileHandler); // dopisz zawartość bufora do pliku
		fflush(fileHandler); // odśwież status pliku
		allRecvData += recvData; // zsumowana ilość pobranych bajtów pliku z serwera
		std::cout << "\r[INFO::CLIENT]\tPobieranie w toku... " << allRecvData << " bajtow." << std::flush;
	}
	std::cout << "\n"; // znak nowej linii
	if (recvData == 0) this->closeConnection(); // przy odebraniu wszystkich danych zamknij połączenie
	fclose(fileHandler); // zamknięcie pliku
	std::cout << "[INFO::CLIENT]\tOdbieranie danych z serwera zakonczone. Pobrano " << allRecvData;
	std::cout << " bajtow. Rozmiar ramki: " << SOCK_NET::Lib::FRAME_BUFF << " bajtow.\n";
}

SOCK_NET::Action SOCK_NET::Client::getAction() const
{
	return this->action; // zwróć aktualną akcję
}

void SOCK_NET::Client::closeConnection()
{
	closesocket(this->sock); // zamknij gniazdo
	this->sock = INVALID_SOCKET; // ustaw gniazdo na nieaktywne
	std::cout << "[INFO::CLIENT]\tRozlaczenie z serwerem " << this->address << ":" << this->port << "\n";
}
