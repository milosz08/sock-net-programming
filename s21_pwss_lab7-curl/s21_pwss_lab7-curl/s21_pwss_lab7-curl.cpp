#pragma comment(lib, "libcurl_imp.lib")
#pragma comment(lib, "jsoncpp.lib")

#include <curl/curl.h>
#include <json/json.h>

#include <iostream>

//Funckja umożliwiająca zapisanie wyniku zawartoœci cURL do stringa, funkcja zapożyczona z 
//https://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) 
{
    ((std::string*)userp)->append((char*)contents, size * nmemb); 
    return size * nmemb; 
}

int main(void)
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    Json::Reader reader;
    Json::Value js;

    curl = curl_easy_init(); // rozpoczêcie sesji
    if (curl) {
        //przekazanie połączenie pod podany adres url
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.open-meteo.com/v1/forecast?latitude=50.30&longitude=18.68&current_weather=true");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); //wywołanie funkcji
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); //wskażnik do wywyo³ania zapisu
        res = curl_easy_perform(curl); //wykonywanie żądania, res otrzymuje kod zwrotny

        //sprawdzenie czy nie wystąpił błąd podczas wykonywania żądania 
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        
        //parsowanie JSONa 
        if (reader.parse(readBuffer, js))
        {
            //walidacja zwracanej temperatury dla podanego API 
                if (js.isObject() && js.isMember("current_weather")) 
                {
                    Json::Value currentWeather = js["current_weather"];
                    if (currentWeather.isObject() && currentWeather.isMember("temperature")) 
                    {
                        double temperature = currentWeather["temperature"].asDouble();
                        std::cout << "Temperatura w aktualnej chwili wynosi: " << temperature << " \370C" << std::endl;
                    }
                    else 
                    {
                        std::cout << "ERROR: obiekt 'current_weather' nie zawiera cz³onu 'temperature'" << std::endl;
                    }
                }
                else 
                {
                    std::cout << "ERROR: JSON nie zawiera obiektu 'current_weather'"<< std::endl;
                }
        }

        curl_easy_cleanup(curl); //wyczyszczenie sesji 
    }
    return 0;
}