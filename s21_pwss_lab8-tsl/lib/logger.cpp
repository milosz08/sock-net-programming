#include "logger.h"

std::pair<std::string, std::string> WSAPI_INIT_SUCCESS = { "INFO", "Inicjalizacja biblioteki WinSock zakończona sukcesem." };
std::pair<std::string, std::string> WSAPI_INIT_FAILURE = { "ERROR", "Inicjalizacja biblioteki WinSock zakończona niepowodzeniem." };
std::pair<std::string, std::string> WSAPI_LIB_INVALID_VERSION = { "ERROR", "Nie znaleziono wymaganej wersji biblioteki WinSock." };

void SOCK_NET::LOGGER::log(const std::pair<std::string, std::string>& message, const std::string file, const int line)
{
	time_t time = std::time(nullptr); // zwróć strukturę czasu
	tm localTime = *std::localtime(&time); // pobierz wartość lokalnego czasu
	// nazwa pliku pobrana z całej ścieżki
	std::string inputTraceFilename = file.substr(file.find_last_of("/\\") + 1);
	// wydrukuj na ekran konsoli zawartość komunikatu
	std::cout
		<< "[" << std::put_time(&localTime, "%d-%m-%Y %H:%M:%S") 
		<< "] [" << message.first << "] ["
		<< inputTraceFilename << ":" << line << "]\t : "
		<< message.second;
}
