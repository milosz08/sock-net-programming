// program serwera - implementacja klasy
#include "server.h"

// konstruktor bezargumentowy wpisujący domyślne wartości adresu i portu do pola klasy
SOCK_NET::Server::Server() : address("127.0.0.1"), port(8080)
{
}

// konstruktor wpisujący wartości adresu i portu podane podczas tworzenia obiektu
SOCK_NET::Server::Server(const char* address, const int& port) : address(address), port(port)
{
}

SOCK_NET::Server::~Server()
{
	this->closeAllConnections();
}

void SOCK_NET::Server::closeAllConnections()
{
	// zamknij wszystkie gniazda klientów i serwera
	for (auto it = this->descrs.begin(); it != this->descrs.end(); it++)
	{
		// jeśli istnieje referencja do pliku, zamknij
		if (this->clients[it->fd].fileHandler != nullptr) fclose(this->clients[it->fd].fileHandler);
		if (it->fd != INVALID_SOCKET) closesocket(it->fd); // zamknij gniazdo, jeśli jest aktywne
	}
	this->closeConnection(); // zamknij główne gniazdo serwera
	this->clients.clear(); // wyczyść mapę klientów
	this->descrs.clear(); // wyczyść wektor deskryptorów
}

bool SOCK_NET::Server::initialize()
{
	struct sockaddr_in sa; // struktura serwera

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli i zwróci false
	if ((this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane utworzenie socketu.");
		return false;
	}
	Lib::log(INFO_L, __FILE__, __LINE__, "Pomyslne utworzenie socketu.");

	sa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	sa.sin_port = htons(this->port); // port serwera

	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów
	if (inet_pton(AF_INET, this->address, &(sa.sin_addr)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane przypisanie IPv4 do struktury.");
		return false;
	}

	// przypisanie struktury do socketu, jeśli zwróci -1 błąd i zwrócenie false
	if (bind(this->sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane powiazanie socketu ze struktura.");
		return false;
	}
	Lib::log(INFO_L, __FILE__, __LINE__, "Powiazanie socketu ze struktura sockaddr_in.");

	// przełączenie socketu w tryb nasłuchiwania połączeń przychodzących poprzez
	// funkcję accept(), jeśli błąd zwróci -1 i zwróci wartość false
	if (listen(this->sock, SOMAXCONN) < 0)
	{
		Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane uruchomienie serwera w trybie nasluchu.");
		return false;
	}

	Lib::log(INFO_L, __FILE__, __LINE__, "Serwer z IP " + std::string(this->address) + " slucha na porcie " 
		+ std::to_string(this->port) + "...");
	// dodanie do wektora struktry reprezentującej serwer
	this->descrs.push_back({ this->sock, POLLIN, 0 });
	return true; // jeśli wszystko ok, zwróć true i uruchom główną pętlę serwera
}

void SOCK_NET::Server::mainLoop()
{
	while (true) // główna pętla serwera
	{
		// tworzenie puli klientów (wraz z serwerem), jeśli nie uda się zwróci błąd i zakończy działanie pęli
		if (WSAPoll(this->descrs.data(), descrs.size(), -1) < 0)
		{
			Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane uruchomienie funkcji WSAPoll.");
			break; // wyjdź z pętli
		}
		pollfd& serv = this->descrs.front(); // weź pierwszy element wektora (deskryptor serwera)
		if (serv.revents & POLLIN) // jeśli jest to serwer
		{
			// spróbuj połączyć z klientem, jeśli się nie uda przejdź do kolejnego klienta
			if (!this->connectWithClient())
			{
				Lib::log(ERROR_L, __FILE__, __LINE__, "Nieudane polaczenie z klientem.");
				Lib::log(INFO_L, __FILE__, __LINE__, "Przejście do nastepnego klienta.");
			}
		}
		// iterator od 2 elementu wektora (pomijanie serwera)
		std::vector<pollfd>::iterator it = this->descrs.begin() + 1;
		while (it != this->descrs.end()) // przejdź przez wszystkich klientów
		{
			if (it->revents & POLLIN) // jeśli klient wysyła dane
			{
				// jeśli klient wysyła nagłówek
				if (clients[it->fd].action == SOCK_NET::Action::HEADER)
				{
					// jeśli wszystko ok, przejdź do kolejnej iteracji
					if (this->readHeader(it->fd, it->events)) it++;
					// w przeciwnym wypadku rozłącz z klientem i przejdź do kolejnego
					else it = this->disconnectWithClient(it);
				}
				// jeśli klient wysyła dane do serwera
				else if (clients[it->fd].action == SOCK_NET::Action::SEND)
				{
					// jeśli wszystko ok, przejdź do kolejnej iteracji
					if (this->recvFileFromClient(it->fd)) it++;
					// w przeciwnym wypadku rozłącz z klientem i przejdź do kolejnego
					else it = this->disconnectWithClient(it);
				}
			}
			// jeśli klient odbiera dane od serwera (serwer wysyła do klienta)
			else if (it->revents & POLLOUT && clients[it->fd].action == SOCK_NET::Action::RECV)
			{
				// jeśli wszystko ok, przejdź do kolejnej iteracji
				if (this->sendFileToClient(it->fd)) it++;
				// w przeciwnym wypadku rozłącz z klientem i przejdź do kolejnego
				else it = this->disconnectWithClient(it);
			}
			// jeśli klient zakończy połączenie lub błąd, rozłącz z klientem
			else if (it->revents & (POLLHUP | POLLERR)) it = this->disconnectWithClient(it);
			else ++it; // w przeciwnym wypadku przejdź do kolejnej iteracji
		}
	}
}

bool SOCK_NET::Server::connectWithClient()
{
	struct sockaddr_in cSa; // struktura klienta wypełniania przez funkcję accept()
	int cSaSize = sizeof(cSa); // wielkość struktury klienta
	SOCKET cSock; // socket nowego klienta

	char clientIp[INET_ADDRSTRLEN]; // adres IP klienta
	// akceptuj połączenie klienta, jeśli napotka błąd zwróci -1 i nie doda do wektora klientów
	if ((cSock = accept(this->sock, (sockaddr*)&cSa, &cSaSize)) < 0)
	{
		closesocket(cSock); // zamknij socket klienta
		return false; // zwróć false, co spowoduje przejście do kolejnego klienta
	}
	// konwersja adresu IP w formie bajtów na ciąg znaków, jeśli nie uda się zwróci NULL
	if ((inet_ntop(AF_INET, &(((struct sockaddr_in*)&cSa)->sin_addr), clientIp, sizeof(clientIp)) == NULL))
	{
		closesocket(cSock); // zamknij socket klienta
		return false; // zwróć false, co spowoduje przejście do kolejnego klienta
	}
	this->descrs.push_back({ cSock, POLLIN, 0 }); // dodaj nowego klienta do wektora
	
	ClientData cData; // stworzenie struktury atrybutów klienta
	cData.address = clientIp;
	cData.clNr = ++this->clientsCount;
	this->clients.insert({ cSock, cData }); // dodaj atrybuty nowego klienta
	this->logDetails(INFO_L, "Udane polaczenie z klientem i dodanie do listy.", cSock, cData, __FILE__, __LINE__);
	return true; // jeśli wszystko ok, zwróć true
}

bool SOCK_NET::Server::readHeader(const SOCKET& cSock, SHORT& cEv)
{
	ClientData& currClient = this->clients[cSock]; // obsługiwany klient
	if (!currClient.headerCollect) // jeśli nie zebrano jeszcze całego nagłówka
	{
		size_t recvSize; // faktyczny rozmiar otrzymanych elementów nagłówka
		char buffer[MAX_BUFF]; // bufor na dane nagłówka
		// pobierz dane nagłówka i umieść w buforze, jeśli zwróci -1 to błąd i zwróć false, w celu rozłączenia
		if ((recvSize = recv(cSock, buffer, MAX_BUFF, 0)) < 0)
		{
			this->logDetails(ERROR_L, "Nieudane odczytanie ramki od klienta.", cSock, currClient, __FILE__, __LINE__);
			return false;
		}
		// dodawanie odczytanych fragmentów do ciągu znaków nagłówka
		for (int i = 0; i < recvSize; i++)
		{
			if (buffer[i] != SEND_INDC && buffer[i] != '\r')
			{
				currClient.header += buffer[i];
				if (SHOW_HDR_PARTS == 1) // jeśli pokazywanie fragmentów nagłówka jest włączone
					this->logDetails(INFO_L, "Kompletowanie naglowka: '" + currClient.header + "'.",
						cSock, currClient, __FILE__, __LINE__);
			}
			else
			{
				this->logDetails(INFO_L, "Odczytano naglowek od klienta.", cSock, currClient, __FILE__, __LINE__);
				currClient.headerCollect = true; // ustaw flagę na odczytane wszystkie dane
				break; // wyjdź z pętli
			}
		}
	}
	if (currClient.headerCollect) // jeśli zebrano cały nagłówek
	{
		if (SHOW_HDR_PARTS == 0) // jeśli pokazywanie fragmentów nagłówka jest wyłączone (pokaż cały nagłówek)
		{
			this->logDetails(INFO_L, "Kompletowanie naglowka: '" + currClient.header + "'.",
				cSock, currClient, __FILE__, __LINE__);
		}
		const size_t delimPos = currClient.header.find(DELIMITER); // pozycja delimitera
		const std::string action = currClient.header.substr(0, delimPos); // typ akcji SEND/RECV
		const std::string fileName = currClient.header.substr(delimPos + 1, currClient.header.size()); // nazwa pliku
		// jeśli nie znaleziono delimitera, rozłącz z klientem i zwróć false co spowoduje przejście do kolejnego
		if (delimPos == std::string::npos)
		{
			this->logDetails(ERROR_L, "Brak delimitera w parametrach naglowka.", cSock, currClient, __FILE__, __LINE__);
			return false; // w przypadku błędu zwróć false
		}
		// sprawdzenie pierwszej części nagłówka, jeśli jest to SEND przejdź do odbierania danych od klienta
		// funkcja strcmpi odpowiadająca za ignorowanie wielkich/małych liter
		if (_strcmpi(action.c_str(), "SEND") == 0)
		{
			// nazwa wynikowa pliku składająca się z IP_KLIENTA_NR_NAZWAPLIKU
			std::string finalFileName = currClient.address + FILE_SEP + std::to_string(currClient.clNr) + FILE_SEP + fileName;

			FILE* fileHandler = fopen(finalFileName.c_str(), "wb"); // utwórz handler z podaną nazwą pliku do zapisu
			if (fileHandler == nullptr) // jeśli nie udało się utworzyć pliku
			{
				this->logDetails(ERROR_L, "Nieudane utworzenie pliku.", cSock, currClient, __FILE__, __LINE__);
				return false; // w przypadku błędu zwróć false
			}
			currClient.fileHandler = fileHandler; // przypisanie uchwytu do struktry klienta
			currClient.fileName = finalFileName; // przypisanie nazwy pliku do struktury klienta
			this->logDetails(INFO_L, "Plik '" + finalFileName + "' zostal utworzony.", cSock, currClient, __FILE__, __LINE__);

			currClient.action = SOCK_NET::Action::SEND; // ustaw flagę na odbieranie danych od klienta
			this->logDetails(INFO_L, "Przelaczenie trybu serwera na tryb SEND.", cSock, currClient, __FILE__, __LINE__);
		}
		else if (_strcmpi(action.c_str(), "RECV") == 0) // w przeciwnym wypadku przejdź do wysyłania danych
		{
			FILE* fileHandler = fopen(fileName.c_str(), "rb"); // utwórz handler z podaną nazwą pliku do odczytu
			if (fileHandler == nullptr) // jeśli nie udało się otworzyć pliku
			{
				this->logDetails(INFO_L, "Proba odwolania sie do nieistniejacego pliku.",
					cSock, currClient, __FILE__, __LINE__);
				return false; // w przypadku błędu zwróć false
			}
			currClient.fileHandler = fileHandler; // przypisanie uchwytu do struktry klienta		
			currClient.fileName = fileName; // przypisanie nazwy pliku do struktury klienta
			currClient.action = SOCK_NET::Action::RECV; // ustaw flagę na wysyłanie danych
			cEv |= POLLOUT; // gotowość wysyłania danych do klienta z serwera
			this->logDetails(INFO_L, "Przelaczenie trybu serwera na tryb RECV.",
				cSock, currClient, __FILE__, __LINE__);
		}
		else // jeśli klient nie podał w nagłówku SEND albo RECV
		{
			this->logDetails(ERROR_L, "Proba wyslania naglowka bez wlasciwych parametrow akcji.",
				cSock, currClient, __FILE__, __LINE__);
			return false; // w przypadku błędu zwróć false
		}
	}
	return true; // jeśli wszystko ok, zwróć true
}

bool SOCK_NET::Server::sendFileToClient(const SOCKET& cSock)
{
	ClientData& currClient = this->clients[cSock]; // obsługiwany klient
	char buffer[FRAME_BUFF]; // bufor na wysyłane dane

	// odczytaj jedną ramkę pliku i zwróć faktycznie odczytaną ilość bajtów
	size_t singleFrameFileSize = fread(buffer, 1, FRAME_BUFF, currClient.fileHandler);
	currClient.sendBytesSize += singleFrameFileSize;
	// jeśli nie odczytano nic, zakończ połączenie z klientem
	if (singleFrameFileSize == 0)
	{
		this->logDetails(INFO_L, "Wyslano " + std::to_string(currClient.sendBytesSize)
			+ " bajtow danych do klienta.", cSock, currClient, __FILE__, __LINE__);
		return false;
	}
	size_t sendData = 0; // ilość wysłanych bajtów
	int sendRes = 0; // ilość bajtów zwróconych przez funckję send()
	size_t remainingData = singleFrameFileSize; // ilość pozostałych danych do wysłania
	do
	{
		// wyślij dane do klienta, jeśli błąd wyświetl komunikat i rozłącz z klientem
		if ((sendRes = send(cSock, (char*)(buffer + sendData), remainingData, 0)) < 0)
		{
			this->logDetails(ERROR_L, "Nieudane wyslanie ramki danych do klienta.",
				cSock, currClient, __FILE__, __LINE__);
			return false;
		}
		sendData += sendRes; // dodaj ilość już wysłanych bajtów
		remainingData -= sendRes; // odejmij ilość pozostałych bajtów do wysłania
	}
	while (remainingData > 0); // wysyłaj dane, dopóki pozostaną jakieś dane do wysłania
	return true;
}

bool SOCK_NET::Server::recvFileFromClient(const SOCKET& cSock)
{
	ClientData& currClient = this->clients[cSock]; // obsługiwany klient
	char buffer[FRAME_BUFF]; // bufor bajtów w rozmiarze ramki

	int recvData = recv(cSock, buffer, FRAME_BUFF, 0); // odbierz dane 
	// jeśli nie uda się odczytać ramki, zakończ połączenie z klientem
	if (recvData < 0)
	{
		this->logDetails(ERROR_L, "Nieudane odczytanie ramki danych.", cSock, currClient, __FILE__, __LINE__);
		return false; // zwróć false i zakończ połączenie z obsługiwanym klientem
	}
	else if (recvData == 0) return false; // zwróć false i zakończ połączenie z obsługiwanym klientem

	fwrite(buffer, sizeof(char), recvData, currClient.fileHandler); // dopisz zawartość bufora do pliku
	fflush(currClient.fileHandler); // odśwież status pliku
	return true; // jeśli wszystko ok, zwróć true
}

std::vector<pollfd>::iterator SOCK_NET::Server::disconnectWithClient(std::vector<pollfd>::iterator& it)
{
	this->logDetails(INFO_L, "Rozlaczono z klientem.", it->fd, this->clients[it->fd], __FILE__, __LINE__);
	// zamknij plik, jeśli istnieje referencja do niego
	if (this->clients[it->fd].fileHandler != nullptr) fclose(this->clients[it->fd].fileHandler);
	closesocket(it->fd); // zamknięcie socketu
	this->clients.erase(it->fd); // usuń klienta z mapy klient-dane
	// usuń deskryptor klienta i zwróć iterator wskazujący na kolejnego klienta
	return this->descrs.erase(it);
}

// metoda drukująca na ekran konsoli dane klienta wraz z dodatkowym komunikatem serwera
void SOCK_NET::Server::logDetails(const std::string& type, const std::string& mess, const SOCKET& cSock,
	const ClientData& cData, const std::string file, const int line) const
{
	std::string clientInfo = "[IP:" + std::string(cData.address) + ", fd: " + std::to_string(cSock)
		+ ", i: " + std::to_string(cData.clNr) + "]";
	Lib::log(type, file, line, mess, true, clientInfo);
}

void SOCK_NET::Server::closeConnection()
{
	closesocket(this->sock); // zamknij socket serwera
	this->sock = INVALID_SOCKET; // ustaw socket na wartość początkową
}