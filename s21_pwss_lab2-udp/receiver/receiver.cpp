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

	receiveFile(); // wywołanie funkcji odbierającej plik od wysyłającego

	WSACleanup(); // zwolnienie zasobów
	system("pause"); // przeciw autmatycznemu wyłączaniu konsoli
	return 0;
}

void receiveFile()
{
	// zainicjalizowanie struktur przechowujących dane połączenia dla wysyłającego i
	// odbierającego
	struct sockaddr_in senderSa, receiverSa;
	int receiverSaSize = sizeof(receiverSa); // wielkość struktury odbierającego
	int sockfd; // zmienna przechowująca deskryptor socketu

	// wywołanie funkcji tworzącej socket typu UDP, jeśli się nie powiedzie, funkcja
	// zwróci wartość -1 i zostanie wyświetlony error w konsoli
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		std::cerr << "Nieudane utworzenie socketu.\n";
		if (closesocket(sockfd) != 0) // zamknięcie socketu
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	senderSa.sin_family = AF_INET; // typ adresu IP wysyłającego (IPv4)
	senderSa.sin_port = htons(PORT); // port wysyłającego (pobierany z definicji preprocesora)
	// przypisz jakikolwiek adres IPv4, dodatkowo wartość przechodzi przez funkcję htonl()
	// tłumaczącą kolejność bajtów procesora na kolejność bajtów sieciowych
	senderSa.sin_addr.s_addr = htonl(INADDR_ANY);

	// połączenie utworzonej struktury wysyłającego z socketem, jeśli się nie powiedzie funkcja
	// zwróci wartość -1 i zostanie wyświetlony błąd
	if (bind(sockfd, (struct sockaddr*)&senderSa, sizeof(senderSa)) < 0)
	{
		std::cerr << "Nieudane polaczenie.\n";
		if (closesocket(sockfd) != 0) // zamknięcie socketu
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	remove(FILE_NAME); // przy uruchomieniu serwera usuń poprzedni plik
	// zmienna przechowująca bufor (pojedynczą ramkę) odbieraną od wysyłającego
	char buffer[FRAME_BUFF];

	int singleFrameSize = 0; // rzeczywista wielkość otrzymanej ramki w kolejnych iteracjach

	// stwórz plik na podstawie nazwy z receiver.h, jeśli się nie powiedzie, wyświetl błąd
	FILE* fileHandler = fopen(FILE_NAME, "wb");
	if (fileHandler == NULL)
	{
		std::cerr << "Nieudane utworzenie pliku.\n";
		if (closesocket(sockfd) != 0) // zamknięcie socketu
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	std::cout << "Odbiorca nasluchuje na plik...\n";
	while (true)
	{
		// pobierz dane od wysyłającego (pojedyncza ramka, na podstawie rozmiaru bufora), w przypadku 
		// zwrócenia -1, oznacza że nie udało się odczytać ramki
		singleFrameSize = recvfrom(sockfd, buffer, FRAME_BUFF, 0, (sockaddr*)&receiverSa, &receiverSaSize);
		if (singleFrameSize < 0) // jeśli błąd, wyświetl komunikat
		{
			std::cerr << "Nieudane odczytanie ramki pliku.\n";
			break;
		}
		else // zapisz do pliku zawartość bufora otrzymaną poprzez funkcję recvfrom
		{
			fwrite(buffer, sizeof(char), singleFrameSize, fileHandler);
		}
	}
	fclose(fileHandler); // zamknięcie pliku

	if (closesocket(sockfd) != 0) // zamknięcie socketu
	{
		std::cerr << "Nieudane zamkniecie socketu.\n";
	}
}