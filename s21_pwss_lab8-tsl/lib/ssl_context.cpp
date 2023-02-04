#include "ssl_context.h"

SOCK_NET::SslContext::SslContext(const SSL_METHOD* sslMethod, const std::string& address, const u_short& port) 
	: sslMethod(sslMethod), address(address), port(port)
{
	WsAPI::logInfo("Inicjalizacja OpenSSL wybrana metoda", __FILE__, __LINE__);
}

SOCK_NET::SslContext::SslContext(const std::string& address, const u_short& port) 
	: sslMethod(TLS_method()), address(address), port(port)
{
	WsAPI::logInfo("Inicjalizacja OpenSSL domyslna metoda: TLS", __FILE__, __LINE__);
}

void SOCK_NET::SslContext::closeSslConnection()
{
	if (this->ssl != nullptr) // jeśli istnieje instancja obiektu SSL
	{
		SSL_shutdown(this->ssl); // zamknij połączenie
		SSL_free(this->ssl); // zwolnij zasoby
		this->ssl = nullptr; // ustaw na pusty wskaźnik
		WsAPI::logInfo("Zamkniecie aktywnego polaczenia SSL", __FILE__, __LINE__);
	}
}

SOCK_NET::SslContext::~SslContext()
{
	this->closeAndFree();
	// jeżeli stworzono instancję kontekstu SSL, zwolnij
	if (this->sslContext != nullptr) SSL_CTX_free(this->sslContext);
	WsAPI::logInfo("Zwolnienie zasobow biblioteki OpenSSL", __FILE__, __LINE__);
}

bool SOCK_NET::SslContext::insertSslContextArgs(int& argc, char** argv)
{
	bool foundCert = false; // flaga informująca o pobraniu certyfikatu z argumentów
	bool foundKey = false; // flaga informująca o pobraniu klucza prywatnego z argumentów
	bool foundCa = false; // flaga informująca o pobraniu certyfikatu głównego
	for (int i = 0; i < argc; i++) // przejdź przez wszystkie podane argumenty
	{
		std::string param = argv[i]; // przypisz do stringa aby móc używać jego metod
		int separatorLocation = param.find('='); // znajdź separator w argumencie
		if (separatorLocation < 0) continue; // jeśli nie znajdzie, przejdź kolejną iterację

		std::string key = param.substr(0, separatorLocation); // klucz
		std::string value = param.substr(separatorLocation + 1, param.size()); // wartość

		if (key == "--sslCertPath") // znajdź paramater przechowujący ścieżkę do certyfikatu
		{
			this->certFile = value;
			foundCert = true; // ustaw flagę na znaleziony
		}
		else if (key == "--sslKeyPath") // znajdź paramater przechowujący ścieżkę do klucza głównego
		{
			this->keyFile = value;
			foundKey = true; // ustaw falgę na znaleziony
		}
		else if (key == "--sslCaPath") // znajdź paramater przechowujący ścieżkę do certyfikatu głównego
		{
			this->caFile = value;
			foundCa = true; // ustaw falgę na znaleziony
		}
	}
	// wypisz komunikaty na ekran konsoli o rezultacie przetwarzania argumentów wejściowych
	if (!foundCert) WsAPI::logError("Nie znaleziono oczekiwanego argumentu '--sslCertPath'", __FILE__, __LINE__);
	else WsAPI::logInfo("Wczytano argument wejsciowy '--sslCertPath=" + this->certFile + "'", __FILE__, __LINE__);
	
	if (!foundKey) WsAPI::logError("Nie znaleziono oczekiwanego argumentu '--sslKeyPath'", __FILE__, __LINE__);
	else WsAPI::logInfo("Wczytano argument wejsciowy '--sslKeyPath=" + this->keyFile + "'", __FILE__, __LINE__);
	
	if (!foundCa) WsAPI::logError("Nie znaleziono oczekiwanego argumentu '--sslCaPath'", __FILE__, __LINE__);
	else WsAPI::logInfo("Wczytano argument wejsciowy '--sslCaPath=" + this->caFile + "'", __FILE__, __LINE__);

	return foundCert && foundKey && foundCa; // jeśli znajdzie wszystkie ścieżki, zwróć true
}

bool SOCK_NET::SslContext::initializeSsl()
{
	if (this->sock != INVALID_SOCKET) // w przypadku aktywnego socketu, wyjdź z metody
	{
		WsAPI::logError("Wybrane polaczenie jest juz aktywne.", __FILE__, __LINE__);
		return false;
	}
	// jeśli nie wywołano metody do ładowania ścieżek certyfikatów z argumentów linii poleceń
	if (this->certFile.empty() || this->caFile.empty() || this->keyFile.empty())
	{
		WsAPI::logError("Aby zainicjalizowac, nalezy wpierw wywolac metode insertSslContextArgs()", __FILE__, __LINE__);
		return false;
	}
	// stwórz nowy kontekst SSL przy użyciu metody przypisanej na etapie tworzenia obiektu
	this->sslContext = SSL_CTX_new(this->sslMethod);
	// wczytaj certyfikat, jeśli coś pójdzie nie tak wypisz komunikat i zwróć false
	if (SSL_CTX_use_certificate_file(this->sslContext, this->certFile.c_str(), SSL_FILETYPE_PEM) != 1)
	{
		WsAPI::logError("Wczytywanie certyfikatu zakonczone niepowodzeniem", __FILE__, __LINE__);
		return false;
	}
	// wczytaj klucz główny, jeśli coś pójdzie nie tak wypisz komunikat i zwróć false
	if (SSL_CTX_use_PrivateKey_file(this->sslContext, this->keyFile.c_str(), SSL_FILETYPE_PEM) != 1)
	{
		WsAPI::logError("Wczytywanie klucza prywatnego zakonczone niepowodzeniem", __FILE__, __LINE__);
		return false;
	}
	// wczytaj certyfikat główny, jeśli coś pójdzie nie tak wypisz komunikat i zwróć false
	if (SSL_CTX_load_verify_locations(this->sslContext, this->caFile.c_str(), nullptr) != 1)
	{
		WsAPI::logError("Wczytywanie certyfikatu glownego zakonczone niepowodzeniem", __FILE__, __LINE__);
		return false;
	}
	// jeśli wszystko ok, przejdź do ustawienia dodatkowych flag
	SSL_CTX_set_verify(this->sslContext, SSL_VERIFY_PEER, nullptr); // weryfikowanie certyfikatu peera
	SSL_CTX_set_options(this->sslContext, SSL_OP_NO_SSLv2); // wyłączenie starej wersji SSLa
	WsAPI::logInfo("Pomyslnie zainicjalizowano certyfikaty i klucz prywatny", __FILE__, __LINE__);
	return true;
}

void SOCK_NET::SslContext::closeAndFree()
{
	this->closeSslConnection();
	if (this->sock != INVALID_SOCKET) // jeśli gniazdo jest aktywne, zamknij
	{
		closesocket(this->sock);
		WsAPI::logInfo("Zamkniecie aktywnego gniazda", __FILE__, __LINE__);
	}
}