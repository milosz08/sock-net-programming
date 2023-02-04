// konteks SSLa - sygnatura klasy
#ifndef SSL_CONTEXT_H
#define SSL_CONTEXT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <curl/curl.h>
#include <json/json.h>

#include "wsapi.h"

#pragma comment(lib, "ws2_32.lib") // zalinkowanie biblioteki WinSock
#pragma comment(lib, "crypt32.lib") // zalinkowanie biblioteki OpenSSL
#pragma comment(lib, "libcurl_imp.lib") // zalinkowanie biblioteki CURL
#pragma comment(lib, "jsoncpp.lib") // zalinkowanie biblioteki JSON

#define FRAME_BUFF_SIZE 1024 // maksymalny rozmiar bufora do wysyłania/odbierania danych

namespace SOCK_NET
{
	class SslContext
	{
		private:
			std::string certFile; // ścieżka do pliku z certyfikatem
			std::string keyFile; // ścieżka do pliku z kluczem prywatnym
			std::string caFile; // ścieżka do pliku z certyfikatem głównym

		protected:
			const SSL_METHOD* sslMethod = nullptr; // wybór metody SSL
			SSL_CTX* sslContext = nullptr; // kontekst SSL
			SSL* ssl = nullptr; // obiekt SSL tworzony na podstawie kontekstu
			SOCKET sock = INVALID_SOCKET; // socket klasy nadrzędnej (klienta/serwera)

			const std::string address; // adres serwera
			const u_short port; // port serwera

			SslContext(const SslContext&) = delete; // usunięcie konstruktora kopiującego
			SslContext(SslContext&&) = delete; // usunięcie konstruktora przenoszącego
			// konstruktor inicjalizujący metodą SSLa
			SslContext(const SSL_METHOD* sslMethod, const std::string& address, const u_short& port);
			SslContext(const std::string& address, const u_short& port); // konstruktor domyślny, inicjalizujący SSLa metodą TLS

			void closeSslConnection(); // zamykanie aktywnego połączenia SSL

		public:
			bool insertSslContextArgs(int& argc, char** argv); // przypisz ścieżki do plików z certyfikatami
			bool initializeSsl(); // inicjalizacja plikami certyfikatów
			void closeAndFree(); // zwolnienie zasobów i zamknięcie połączenia

			SslContext& operator= (const SslContext&) = delete; // usunięcie operatora przypisania
			SslContext& operator= (SslContext&&) = delete; // usunięcie operatora kopiowania
			virtual ~SslContext(); // destruktor zwalniający zasoby
	};
}

#endif // SSL_CONTEXT_H