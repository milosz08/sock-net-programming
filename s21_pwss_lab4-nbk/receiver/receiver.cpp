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
	std::map<SOCKET, FILE*> clients; // lista klientów
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

	descrs.push_back({ socketDesc, POLLIN, 0 });
	std::cout << "Serwer slucha...\n";
	while (true) // główna pętla serwera
	{
		struct sockaddr_in clientSa; // struktura klienta wypełniania przez funkcję accept()
		int clientSaLength = sizeof(clientSa); // wielkość struktury klienta
		
		if (WSAPoll(descrs.data(), descrs.size(), -1) < 0)
		{
			std::cerr << "Sadeg nie udalo sie WSAcostam\n";
			break;
		}

		pollfd& serverDesc = descrs.front();
		if (serverDesc.revents & POLLIN)
		{
			if ((clientDesc = accept(socketDesc, (sockaddr*)&clientSa, &clientSaLength)) < 0)
			{
				std::cerr << "Blad podczas polaczenia z klientem.\n";
				closesocket(clientDesc); // zamknięcie socketu
				break; // wyjście z głównej pętli serwera
			}
			// PULLHUP nie jest wymagane ale na innych SO może być potrzebne
			descrs.push_back({ clientDesc, POLLIN | POLLHUP, 0 });
			clientsCount++;
		}

		std::vector<pollfd>::iterator next = descrs.begin() + 1;
		while (next != descrs.end())
		{
			char clientIp[INET_ADDRSTRLEN];
			// konwersja adresu IP w formie bajtów na ciąg znaków, jeśli nie uda się zwróci NULL
			if ((inet_ntop(AF_INET, &(((struct sockaddr_in*)&clientSa)->sin_addr), clientIp, sizeof(clientIp)) == NULL))
			{
				std::cerr << "Nieudane odczytanie adresu IPv4 ze struktury sockaddr_in.\n";
				closesocket(descrs[next->fd].fd); // zamknięcie socketu
				break; // wyjście z głównej pętli serwera
			}
			std::cout << "Polaczono z klientem z IP " << clientIp << "\n";

			// wynikowa nazwa pliku, w formacie IP__FILE_NAME
			const int clientItCharCount = sizeof(std::to_string(clientsCount).c_str()) / sizeof(char);
			char fileName[INET_ADDRSTRLEN + sizeof(FILE_NAME) / sizeof(char) + 2 + clientItCharCount];
			strcpy(fileName, clientIp); // dodanie do tablicy znaków IP klienta
			strcat(fileName, "_"); // dodanie separatora
			strcat(fileName, std::to_string(clientsCount).c_str()); // dodanie numeru klienta
			strcat(fileName, "_"); // dodanie separatora
			strcat(fileName, FILE_NAME); // dodanie nazwy pliku pobranej z receiver.h

			remove(fileName); // usuń wcześniejszy plik
			char buffer[FRAME_BUFF]; // bufor bajtów w rozmiarze ramki

			int singleFrameSize = 0; // ilość bajtów pojedynczej ramki
			// stwórz plik na podstawie nazwy z receiver.h, jeśli się nie powiedzie, błąd
			FILE* fileHandler = fopen(fileName, "wb");
			if (fileHandler == NULL)
			{
				std::cerr << "Nieudane otwarcie pliku " << FILE_NAME << ".\n";
				break;
			}

			SOCKET currClientDescr = next->fd;
			clients.insert({ currClientDescr, fileHandler });

			while (true)
			{
				// pobierz dane od wysyłającego (pojedyncza ramka, na podstawie rozmiaru bufora), w przypadku 
				// zwrócenia -1, oznacza że nie udało się odczytać ramki
				singleFrameSize = recv(currClientDescr, buffer, FRAME_BUFF, 0);
				if (singleFrameSize < 0)
				{
					std::cerr << "Nieudane odczytanie ramki pliku. Brak polaczenia z klientem.\n";
					fclose(fileHandler); // zamknięcie pliku
					remove(fileName); // usunięcie pliku
					break; // wyjście z pętli odczytującej bajty pliku
				}
				else if (singleFrameSize == 0) // jeśli liczba bajtów wynosi 0, zakończ
				{
					std::cout << "Przechwytywanie zakonczone. Plik wynikowy: " << fileName << "\n";
					fclose(fileHandler); // zamknięcie pliku
					break; // wyjście z pętli odczytującej bajty pliku
				}
				// zapisz do pliku zawartość bufora otrzymaną poprzez funkcję recv
				fwrite(buffer, sizeof(char), singleFrameSize, fileHandler);
			}
			fclose(fileHandler); // zamknięcie pliku
			closesocket(currClientDescr); // zamknięcie socketu (rozłączenie z klientem)
			clients.erase(next->fd);
			descrs.erase(next);
			next++;
			std::cout << "Rozlaczono z klientem z IP " << clientIp << "\n";
		}
	}
	closesocket(socketDesc); // zamknięcie socketu
}