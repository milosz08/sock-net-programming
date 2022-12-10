// receiver.cpp
#include "receiver.h"


int main(int argc, char** argv)
{
	WSADATA wsaData; // zainicjalizowanie struktury WSADATA
	// uruchomienie biblioteki WinSock, jeśli błąd wyświetl komunikat i zakończ działanie
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Uruchomienie zakończone niepowodzeniem.\n";
		return 1;
	}

	receiveFile(); // wywołanie funkcji odbierającej plik od klienta

	WSACleanup(); // zwolnienie zasobów
	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}

void receiveFile()
{
	struct sockaddr_in recvSa; // struktura serwera
	SOCKET socketDesc, clientDesc; // socket serwera i socket klienta
	std::vector<pollfd> descrs; // wektor deskryptorów socketów
	std::map<SOCKET, FILE*> fileClients; // mapa identyfikująca klienta z socketem
	int clientsCount = 0; // licznik klientów

	// wywołanie funkcji tworzącej socket typu TCP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli
	if ((socketDesc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		std::cerr << "Nieudane utworzenie socketu.\n";
		closesocket(socketDesc); // zamknięcie socketu
		return;
	}
	
	recvSa.sin_family = AF_INET; // typ adresu IP serwera (IPv4)
	recvSa.sin_port = htons(PORT); // port serwera (pobierany z definicji preprocesora)
	// przypisz do struktury adres serwera przy pomocy funkcji inet_pton() która konwertuje
	// zapis w postaci tablicy znaków na sieciową kolejność bajtów
	if (inet_pton(AF_INET, RECV_INET, &(recvSa.sin_addr)) < 0)
	{
		std::cerr << "Podany adres: " << RECV_INET << " nie jest prawidlowym adresem IPv4.\n";
		closesocket(socketDesc); // zamknięcie socketu
		return;
	}

	// przypisanie struktury do socketu, jeśli zwróci -1 błąd i zakończenie działania
	if (bind(socketDesc, (struct sockaddr*)&recvSa, sizeof(recvSa)) < 0)
	{
		std::cerr << "Blad podczas powiazywania socketu z struktura.\n";
		closesocket(socketDesc); // zamknięcie socketu
		return;
	}

	// przełączenie socketu w tryb nasłuchiwania połączeń przychodzących poprzez
	// funkcję accept, jeśli błąd zwróci -1 i zakończy działanie serwera
	if (listen(socketDesc, SOMAXCONN) < 0)
	{
		std::cerr << "Blad podczas uruchomienia serwera.\n";
		closesocket(socketDesc); // zamknięcie socketu
		return;
	}

	descrs.push_back({ socketDesc, POLLIN, 0 }); // dodanie do wektora serwera
	std::cout << "Serwer slucha...\n";
	while (true) // główna pętla serwera
	{
		struct sockaddr_in clientSa; // struktura klienta wypełniania przez funkcję accept()
		int clientSaLength = sizeof(clientSa); // wielkość struktury klienta

		// tworzenie puli klientów (wraz z serwerem), jeśli nie uda się zwróci błąd i zakończy działanie pęli
		if (WSAPoll(descrs.data(), descrs.size(), -1) < 0)
		{
			std::cerr << "Blad podczas tworzenia WSAPoll\n";
			break;
		}

		pollfd& serv = descrs.front(); // weź pierwszy element wektora (deskryptor serwera)

		// jeśli jest to serwer
		if (serv.revents & POLLIN)
		{
			// akceptuj połączenie klienta, jeśli napotka błąd zwróci -1 i zakończy działanie pętli serwera
			if ((clientDesc = accept(socketDesc, (sockaddr*)&clientSa, &clientSaLength)) < 0)
			{
				std::cerr << "Blad podczas polaczenia z klientem.\n";
				closesocket(clientDesc); // zamknięcie socketu
				continue; // w przypadku błędu, kontynuuj działanie serwera
			}

			char clientIp[INET_ADDRSTRLEN]; // adres IP klienta
			// konwersja adresu IP w formie bajtów na ciąg znaków, jeśli nie uda się zwróci NULL
			if ((inet_ntop(AF_INET, &(((struct sockaddr_in*)&clientSa)->sin_addr), clientIp, sizeof(clientIp)) == NULL))
			{
				std::cerr << "Nieudane odczytanie adresu IPv4 ze struktury sockaddr_in.\n";
				closesocket(clientDesc);
				continue; // w przypadku błędu, kontynuuj działanie serwera
			}
			std::cout << "Polaczono z klientem z IP " << clientIp << ". Nr deskryptora " << clientDesc << "\n";

			std::string separator = "__";
			std::string finalFileName = clientIp + separator + std::to_string(clientsCount) + separator + FILE_NAME;

			// stwórz plik na podstawie nazwy z receiver.h, jeśli się nie powiedzie, błąd
			FILE* fileHandler = fopen(finalFileName.c_str(), "wb");
			if (fileHandler == NULL)
			{
				std::cerr << "Nieudane otwarcie pliku " << finalFileName << ".\n";
				closesocket(clientDesc);
				continue; // w przypadku błędu, kontynuuj działanie serwera
			}

			fileClients.insert({ clientDesc, fileHandler }); // dodaj właściwość klient-plik do mapę
			descrs.push_back({ clientDesc, POLLIN, 0 }); // dodaj klienta do wektora
			clientsCount++; // inkrementacja klientów
		}

		// iterator od 2 elementu wektora (pomijanie serwera)
		std::vector<pollfd>::iterator i = descrs.begin() + 1;
		while (i != descrs.end()) // przejdź przez wszystkich klientów
		{
			// jeśli klient wysyła dane
			if (i->revents & POLLIN)
			{
				char buffer[FRAME_BUFF]; // bufor bajtów w rozmiarze ramki
				int singleFrameSize = recv(i->fd, buffer, FRAME_BUFF, 0);
				if (singleFrameSize < 0)
				{
					std::cerr << "Nieudane odczytanie ramki pliku.\n";
					continue; // w przypadku błędu, kontynuuj działanie serwera
				}
				// dopisz do pliku otrzymaną ramkę danych
				fwrite(buffer, sizeof(char), singleFrameSize, fileClients.at(i->fd));
				fflush(fileClients.at(i->fd)); // odśwież status pliku
				++i;
			}
			// jeśli klient zakończy połączenie lub błąd
			else if (i->revents & (POLLHUP | POLLERR))
			{
				std::cout << "Rozlaczono z klientem (nr deskryptora): " << i->fd << "\n";
				fclose(fileClients.at(i->fd)); // zamknięcie pliku
				closesocket(i->fd); // zamknięcie socketu
				fileClients.erase(i->fd); // usuń klienta z mapy klient-plik
				i = descrs.erase(i); // usuń deskryptor klienta i zwróć iterator
			}
			else ++i; // przejdź do kolejnego klienta
		}
	}
	closesocket(socketDesc); // zamknięcie socketu
}