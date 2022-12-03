// sender.cpp
#include "sender.h"

int main(int argc, char** argv)
{
	WSADATA wsaData; // zainicjalizowanie struktury WSADATA
	// uruchomienie biblioteki WinSock, jeśli błąd wyświetl komunikat i zakończ działanie
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Uruchomienie zakończone niepowodzeniem.\n";
		return 1;
	}

	sendFile(); // wywołanie funkcji odbierającej plik od wysyłającego

	WSACleanup(); // zwolnienie zasobów
	system("pause"); // przeciw automatycznemu wyłączaniu konsoli
	return 0;
}

void sendFile()
{
	struct sockaddr_in recvSa; // struktura do połączenia się z serwerem
	SOCKET socketDesc; // deskryptor socketu

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

	// zainicjalizuj połączenie z serwerem, jeśli nie uda się zwróci -1 i zakończy działanie
	if (connect(socketDesc, (sockaddr*)&recvSa, sizeof(recvSa)) < 0)
	{
		std::cerr << "Nie udalo sie polaczyc z serwerem.\n";
		closesocket(socketDesc); // zamknięcie socketu
		return;
	}

	std::cout << "Nawiazano polaczenie z serwerem z IP " << RECV_INET << "\n";

	// odczytaj plik na podstawie nazwy z sender.h, jeśli się nie powiedzie, wyświetl błąd
	FILE* fileHandler = fopen(FILE_NAME, "rb");
	if (fileHandler == NULL)
	{
		std::cerr << "Nieudane otwarcie pliku " << FILE_NAME << ".\n";
		closesocket(socketDesc); // zamknięcie socketu
		return;
	}

	// zmienna przechowująca bufor (pojedynczą ramkę) odbieraną od wysyłającego
	char buffer[FRAME_BUFF];

	// ustaw pozycję kursora na końcu pliku
	fseek(fileHandler, 0, SEEK_END);
	// na podstawie bieżącej pozycji pobierz ilość bajtów pliku
	int fileFullSize = ftell(fileHandler);
	int frames = 0; // ilość wysłanych ramek
	float totalElapsed = 0; // czas wysyłania danych
	
	// pętla iterująca do momentu, kiedy prześle wszystkie bajty (ilość ramek * jej rozmiar)
	while ((frames * FRAME_BUFF) < fileFullSize)
	{
		// przesuń pozycję kursora na podstawie pobieranej ramki (ilość pobranych ramek * jej rozmiar),
		fseek(fileHandler, (frames * FRAME_BUFF), SEEK_SET);
		// odczytaj jedną ramkę pliku i zwróć faktycznie odczytaną ilość bajtów
		size_t singleFrameFileSize = fread(buffer, 1, FRAME_BUFF, fileHandler);

		const clock_t begin_time = clock(); // rozpocznij odliczanie czasu

		if (send(socketDesc, buffer, singleFrameFileSize, 0) < 0)
		{
			std::cerr << "Nieudane wyslanie danych.\n";
			closesocket(socketDesc); // zamknięcie socketu
			fclose(fileHandler); // zamknięcie pliku
			return;
		}

		// zakończ odliczanie czasu i zsumuj
		totalElapsed += float(clock() - begin_time) / CLOCKS_PER_SEC;
		// wyświetl procent wysłania danych przez klienta
		//float percentage = ((++frames * FRAME_BUFF) / (float)fileFullSize) * 100;
		//std::cout << "\rWysylanie w toku... " << (int)percentage << "%" << std::flush;
		++frames;
	}
	fclose(fileHandler); // zamknięcie pliku

	std::cout << "\nWyslano " << frames << " ramek, kazda po " << FRAME_BUFF << " bajtow.\n";
	std::cout << "Czas wysylania pliku: " << totalElapsed << " sek.\n";
	std::cout << "Calkowity rozmiar pliku: " << fileFullSize << " bajtow.\n";

	closesocket(socketDesc); // zamknięcie socketu
	std::cout << "Rozlaczono z serwerem z IP " << RECV_INET << "\n";
}