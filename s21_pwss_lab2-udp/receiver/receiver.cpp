#include "receiver.h"

int main(int argc, char** argv)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "Uruchomienie zakończone niepowodzeniem.\n";
		return 1;
	}

	receiveFile();

	WSACleanup();
	return 0;
}

void receiveFile()
{
	struct sockaddr_in senderSa, receiverSa;
	int receiverSaSize = sizeof(receiverSa);
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		std::cerr << "Nieudane utworzenie socketu.\n";
		return;
	}

	senderSa.sin_family = AF_INET;
	senderSa.sin_port = htons(PORT);
	senderSa.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*)&senderSa, sizeof(senderSa)) < 0)
	{
		std::cerr << "Nieudane polaczenie.\n";
		if (closesocket(sockfd) != 0)
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	remove(FILE_NAME);

	char buffer[FRAME_BUFF];

	int singleFrameSize = 0;
	FILE* fileHandler = fopen(FILE_NAME, "wb");

	std::cout << "Odbiorca nasluchuje na plik...\n";
	while (true)
	{
		singleFrameSize = recvfrom(sockfd, buffer, FRAME_BUFF, 0, (sockaddr*)&receiverSa, &receiverSaSize);
		if (singleFrameSize < 0)
		{
			std::cerr << "Nieudane odczytanie ramki pliku.\n";
			if (closesocket(sockfd) != 0)
			{
				std::cerr << "Nieudane zamkniecie socketu.\n";
			}
			break;
		}
		fwrite(buffer, sizeof(char), singleFrameSize, fileHandler);
	}
	fclose(fileHandler);

	if (closesocket(sockfd) != 0)
	{
		std::cerr << "Nieudane zamkniecie socketu.\n";
	}
}