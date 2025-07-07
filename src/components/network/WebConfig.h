#include "NetworkManager.h"
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "../storage/storage.h"
#include "CaptiveHandler.h"
#include "../utils/ESPUtils.h"

// Asegúrate de que esta estructura es segura para transmisión binaria
struct WifiCredentials
{
    char ssid[32];
    char password[32];
    char ip[16];   // Formato "192.168.4.1"
    char mask[16]; // Formato "255.255.255.0"
    char gtw[16];  // Formato "192.168.4.1"
};

class WebConfigServer
{
private:
    NetworkManager *networkManager;
    Storage *storageManager;
    ModuleInfo myModule;
    AsyncEventSource *events;
    DNSServer dnsServer;
    AsyncWebServer server;

    char *ssid = new char[32]; // Asegúrate de que el tamaño es suficiente para el SSID
    const char *password = ""; // Asegúrate de que el tamaño es suficiente para la contraseña

    IPAddress localIp = IPAddress(192, 168, 4, 1);
    IPAddress gateway = IPAddress(192, 168, 4, 1);
    IPAddress subnet = IPAddress(255, 255, 255, 0);

    // Server Loop Task
    static void serverLoopTask(void *pvParameters)
    {
        WebConfigServer *instance = static_cast<WebConfigServer *>(pvParameters);
        JsonDocument jsonDoc;
        JsonArray networksArray = jsonDoc.to<JsonArray>();

        SystemState lastState = SystemState::IDLE;
        while (true)
        {
            // Procesa los Eventos del Modulo
            SystemState currentState = getCurrentState();
            if (currentState != lastState)
            {
                if (currentState == SystemState::ERROR_NETWORK_TIMEOUT)
                {
                    JsonDocument networkDoc;
                    networkDoc["status"] = "ERROR_NETWORK_TIMEOUT";
                    instance->events->send(networkDoc.as<String>().c_str(), "status", millis());
                }
                else if (currentState == SystemState::WIFI_GOT_IP)
                {
                    JsonDocument networkDoc;
                    networkDoc["status"] = "GOT_IP";
                    networkDoc["ip"] = instance->networkManager->getLocalIP();
                    instance->events->send(networkDoc.as<String>().c_str(), "status", millis());
                }

                lastState = currentState;
            }

            uint8_t scanCount = instance->networkManager->scanNetworksComplete(&networksArray, 10);
            if (scanCount > 0)
            {
                jsonDoc["status"] = "SCAN_COMPLETED";
                jsonDoc["networks"] = networksArray;
                instance->events->send(jsonDoc.as<String>().c_str(), "scan", millis());
                changeState(SystemState::WIFI_SCANNING_COMPLETED);
            }

            // Procesa las solicitudes DNS
            instance->dnsServer.processNextRequest();
            vTaskDelay(100 / portTICK_PERIOD_MS); // Reduce la carga del CPU
            esp_task_wdt_reset();                 // Resetea el watchdog para evitar timeouts
        }
    }

    String processor(const String &var);
    void notFound(AsyncWebServerRequest *request);
    void scanNetwork(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void connectNetwork(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void configure(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
    void restart(AsyncWebServerRequest *request)
    {
        request->send_P(200, "application/json", "{\"status\":\"ok\"}");
        ESP.restart();
    }

public:
    WebConfigServer(NetworkManager *networkManager, Storage *storageManager);
    ~WebConfigServer();

    void begin(ModuleInfo &myModule);
};

WebConfigServer::WebConfigServer(NetworkManager *networkManager, Storage *storageManager)
    : networkManager(networkManager), storageManager(storageManager), server(80)
{
    sprintf(ssid, "SoiLion_Module_%s", ESPUtils::getDeviceId());
    events = new AsyncEventSource("/events");
}

WebConfigServer::~WebConfigServer()
{
    delete[] ssid;
    delete[] password;
    delete events;
}

void WebConfigServer::begin(ModuleInfo &myModule)
{
    if (!LittleFS.begin(true))
    {
        Serial.println("Error al montar LittleFS");
        return;
    }

    networkManager->initializeAP(ssid, password, localIp, gateway, subnet);

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", String(), false,
                              [this](const String &var)
                              { return this->processor(var); }); });

    server.on("/network", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/network.html", String(), false,
                              [this](const String &var)
                              { return this->processor(var); }); });

    server.on("/boards", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/boards.html", String(), false,
                              [this](const String &var)
                              { return this->processor(var); }); });

    server.on("/success", HTTP_GET, [this](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/setup_success.html", String(), false, [this](const String &var)
                              { return this->processor(var); }); });

    server.on("/pending", HTTP_GET,
              [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/pages/setup_failed.html"); });

    server.on("/styles/network.css", HTTP_GET,
              [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles/network.css", "text/css"); });

    server.on("/styles/server.css", HTTP_GET,
              [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles/server.css", "text/css"); });

    server.on("/styles/success.css", HTTP_GET,
              [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/styles/success.css", "text/css"); });

    server.on("/scripts/network.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/scripts/network.js", "application/javascript"); });

    server.on("/scripts/configure.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/scripts/configure.js", "application/javascript"); });

    server.on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->scanNetwork(request, nullptr, 0, 0, 0); });

    server.on("/restart", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->restart(request); });

    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { this->connectNetwork(request, data, len, index, total); });

    server.on("/configure", HTTP_POST, [](AsyncWebServerRequest *request) {}, nullptr, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { this->configure(request, data, len, index, total); });

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(404, "text/plain", "Not Found"); });

    server.addHandler(events); // 👈 habilita SSE
    server.addHandler(new CaptiveRequestHandler());
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.setTTL(300);
    dnsServer.start(53, "*", localIp); // Configura el servidor DNS para redirigir todas las solicitudes a la IP local
    server.begin();

    xTaskCreatePinnedToCore(
        serverLoopTask, "ServerLoop", 4096, this, 1, NULL, 1); // Crea la tarea del bucle del servidor en el núcleo 1
}

String WebConfigServer::processor(const String &var)
{

    if (var == "ID_MODULE")
    {
        return String(ESPUtils::getDeviceId());
    }
    else if (var == "WIFI_SSID")
    {
        return String(myModule.wifiSSID);
    }
    else if (var == "WIFI_STATIC_IP")
    {
        return String(myModule.staticIp.toString());
    }
    else if (var == "WIFI_SUBNET_MASK")
    {
        return String(myModule.subnetMask.toString());
    }
    else if (var == "WIFI_GATEWAY")
    {
        return String(myModule.gateway.toString());
    }
    return String("Error");
}

void WebConfigServer::notFound(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(404, "text/plain", "Not found");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void WebConfigServer::scanNetwork(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    networkManager->scanNetworks();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/json", "{\"status\":\"ok\"}");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void WebConfigServer::connectNetwork(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    Serial.println("🔌 Parsing network credentials from JSON...");

    if (!data || len == 0)
    {
        request->send(400, "text/plain", "No data received");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error)
    {
        Serial.printf("❌ JSON parse failed: %s\n", error.c_str());
        request->send(400, "text/plain", "Invalid JSON format");
        return;
    }

    myModule.wifiSSID = doc["ssid"].as<String>();
    myModule.wifiPassword = doc["password"].as<String>();

    String ipStr = doc["ip"].as<String>();
    IPAddress ip;
    if (ip.fromString(ipStr))
        myModule.staticIp = ip;

    ipStr = doc["mask"].as<String>();
    if (ip.fromString(ipStr))
        myModule.subnetMask = ip;

    ipStr = doc["gtw"].as<String>();
    if (ip.fromString(ipStr))
        myModule.gateway = ip;

    if (!myModule.wifiSSID || !myModule.wifiPassword)
    {
        request->send(400, "text/plain", "Missing SSID or password");
        return;
    }

    if (!heap_caps_check_integrity_all(true))
    {
        Serial.println("🧨 Heap corruption detected BEFORE connect");
        request->send(500, "text/plain", "Heap error before connect");
        return;
    }

    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/json", networkManager->connect(networkType::WIFI, myModule.wifiSSID.c_str(), myModule.wifiPassword.c_str()) ? "{\"status\":\"Connecting\"}" : "{\"status\":\"Connection failed\"}");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void WebConfigServer::configure(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    Serial.println("Configuring module...");
    JsonDocument configDoc;
    DeserializationError error = deserializeJson(configDoc, data, len);

    if (error)
    {
        Serial.printf("❌ JSON parse failed: %s\n", error.c_str());
        request->send(400, "text/plain", "Invalid JSON format");
        return;
    }

    myModule.boardType = configDoc["boardType"].as<String>();
    Serial.printf("✅ Configuration saved: %s\n", configDoc.as<String>().c_str());

    if (storageManager->writeInformation(myModule))
    {
        Serial.println("✅ Configuration saved, Board Type: " + myModule.boardType);
        changeState(SystemState::CONFIGURATION_SUCCESS);
        AsyncWebServerResponse *response = request->beginResponse_P(200, "application/json", "{\"status\":\"ok\"}");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    }
    else
    {
        Serial.println("❌ Error al guardar la configuracion");
        changeState(SystemState::ERROR_CONFIGURATION_FAILED);
        AsyncWebServerResponse *response = request->beginResponse_P(500, "application/json", "{\"status\":\"failed\"}");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    }
}