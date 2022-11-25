#include "sender.h"

int main(int argc, char** argv)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Uruchomienie zakończone niepowodzeniem.\n";
		return 1;
	}

	sendFile();

	WSACleanup();
	system("pause");
	return 0;
}

void sendFile()
{
	struct sockaddr_in receiverSa;
	int iResult;

	if ((iResult = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		std::cerr << "Nieudane utworzenie socketu.\n";
		return;
	}

	receiverSa.sin_family = AF_INET;
	receiverSa.sin_port = htons(PORT);
	if (inet_pton(AF_INET, RECV_INET, &(receiverSa.sin_addr)) < 0)
	{
		std::cerr << "Podany adres: " << RECV_INET << " nie jest prawidlowym adresem IPv4.\n";
		if (closesocket(iResult) != 0)
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	if (connect(iResult, (sockaddr*)&receiverSa, sizeof(receiverSa)) < 0) {
		std::cerr << "Nieudane połączenie\n";
		if (closesocket(iResult) != 0)
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	FILE* fileHandler = fopen(FILE_NAME, "rb");
	if (fileHandler == NULL)
	{
		std::cerr << "Nieudane otwarcie pliku " << FILE_NAME << ".\n";
		if (closesocket(iResult) != 0)
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	char buffer[FRAME_BUFF];

	fseek(fileHandler, 0, SEEK_END);
	int fileFullSize = ftell(fileHandler);
	int frames = 0;

	const clock_t begin_time = clock();

	while ((frames * FRAME_BUFF) < fileFullSize)
	{
		fseek(fileHandler, (frames * FRAME_BUFF), SEEK_SET);
		size_t singleFrameFileSize = fread(buffer, 1, FRAME_BUFF, fileHandler);

		if (send(iResult, buffer, singleFrameFileSize, 0) < 0)
		{
			std::cerr << "Nieudane wysłanie danych.\n";
			if (closesocket(iResult) != 0)
			{
				std::cerr << "Nieudane zamkniecie socketu.\n";
			}
			fclose(fileHandler);
			return;
		}
		frames++;
	}
	
	float elapseTime = float(clock() - begin_time) / CLOCKS_PER_SEC;

	std::cout << "Wyslano " << frames << " ramek, kazda po " << FRAME_BUFF << " bajtow.\n";
	std::cout << "Czas wysylania pliku: " << elapseTime << " sek.\n";
	std::cout << "Calkowity rozmiar pliku: " << fileFullSize << " bajtow.\n";

	if (closesocket(iResult) != 0)
	{
		std::cerr << "Nieudane zamkniecie socketu.\n";
	}
}