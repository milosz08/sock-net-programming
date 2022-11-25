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
	system("pause");
	return 0;
}

void receiveFile()
{
	struct sockaddr_in senderSa, receiverSa;
	int receiverSaSize = sizeof(receiverSa);
	int listenSocket, iResult, clientSocket;

	if ((listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		std::cerr << "Nieudane utworzenie socketu.\n";
		return;
	}

	senderSa.sin_family = AF_INET;
	senderSa.sin_port = htons(PORT);
	senderSa.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((iResult = bind(listenSocket, (struct sockaddr*)&senderSa, sizeof(senderSa))) < 0)
	{
		std::cerr << "Nieudane polaczenie.\n";
		if (closesocket(iResult) != 0 || closesocket(listenSocket) != 0)
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
		}
		return;
	}

	if ((iResult = listen(listenSocket, SOMAXCONN)) < 0) {
		std::cerr << "Nie potrafie sluchac. Sadeg.\n";
		if (closesocket(iResult) != 0 || closesocket(listenSocket) != 0)
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
		if ((clientSocket = accept(listenSocket, NULL, NULL)) < 0) {
			std::cerr << "Nie potrafie sie polaczyc. Sadeg.\n" << WSAGetLastError();
			if (closesocket(iResult) != 0 || closesocket(listenSocket) != 0 || closesocket(clientSocket) != 0)
			{
				std::cerr << "Nieudane zamkniecie socketu.\n";
			}
			return;
		}

		while (true)
		{
			singleFrameSize = recv(clientSocket, buffer, FRAME_BUFF, 0);
			if (singleFrameSize == 0)
			{
				std::cout << "Wyslano plik.\n";
				fclose(fileHandler);
				break;
			}
			if (singleFrameSize < 0)
			{
				std::cerr << "Nieudane odczytanie ramki pliku.\n";
				if (closesocket(iResult) != 0 || closesocket(clientSocket) != 0)
				{
					std::cerr << "Nieudane zamkniecie socketu.\n";
				}
				break;
			}
			fwrite(buffer, sizeof(char), singleFrameSize, fileHandler);
		}
		fclose(fileHandler);

		if (closesocket(clientSocket) != 0)
		{
			std::cerr << "Nieudane zamkniecie socketu.\n";
			return;
		}
	}

	if (closesocket(iResult) != 0 || closesocket(clientSocket) != 0)
	{
		std::cerr << "Nieudane zamkniecie socketu.\n";
	}
}