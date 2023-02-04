// program serwera - sygnatura klasy
#ifndef SERVER_H
#define SERVER_H

#include <lib\wsapi.h>
#include <lib\ssl_context.h>

namespace SOCK_NET
{
	class Server : public SslContext
	{
		public:
			Server(const Server&) = delete; // usunięcie konstruktora kopiującego
			Server(Server&&) = delete; // usunięcie konstruktora przenoszącego
			Server(); // konstruktor domyślny, inicjalizujący domyślnym adresem i portem
			Server(const std::string& address, const u_short& port); // konstruktor inicjalizujący serwer
			~Server(); // destruktor uruchamiany po zakończeniu pracy serwera
			
			Server& operator= (const Server&) = delete; // usunięcie operatora przypisania (kopiowanie)
			Server& operator= (Server&&) = delete; // usunięcie operatora przypisania (przenoszenie)

			bool initializeServer(); // inicjalizacja serwera (socket, bind, listen itp.)
			void receiveData(); // główna pętla serwera przyjmująca połączenia klientów

		private:
			// dekodowanie ciągu znaków na format JSON i wyświetlanie na ekranie
			static void decodeJsonDataAndPrint(std::string& data);
			void disconnectWithClient(char* ip, SOCKET& sock); // rozłączenie z klientem
	};
}

#endif // SERVER_H