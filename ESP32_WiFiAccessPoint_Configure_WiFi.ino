#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// #include <EEPROM.h>

#include "HTMLForm.h" // HTML Form
#include "wifi_config.h"

const char* SOFT_AP_SSID = "ESP32";
const char* SOFT_AP_PSK = "12345678"; // Must be 8 characters or more

char ssid[32] = "";
char password[63] = "";
volatile boolean g_is_wifi_configured = false;
volatile boolean g_is_wifi_client_connected = false;

AsyncWebServer server(80);

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case SYSTEM_EVENT_WIFI_READY: 
      // Serial.println("WiFi interface ready");
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      // Serial.println("Completed scan for access points");
      break;
    case SYSTEM_EVENT_STA_START:
      // Serial.println("WiFi client started");
      break;
    case SYSTEM_EVENT_STA_STOP:
      // Serial.println("WiFi clients stopped");
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      // Serial.println("Connected to access point");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      // Serial.println("Disconnected from WiFi access point");
      // WiFi.begin(ssid, password);
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      // Serial.println("Authentication mode of access point has changed");
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      // Serial.print("Obtained IP address: ");
      // Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      // Serial.println("Lost IP address and IP address is reset to 0");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      // Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      // Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      // Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      // Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
    case SYSTEM_EVENT_AP_START:
      // Serial.println("WiFi access point started");
      break;
    case SYSTEM_EVENT_AP_STOP:
      // Serial.println("WiFi access point  stopped");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      // Serial.println("Client connected");
      // Set the flag when a client connects
      g_is_wifi_client_connected = true;
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      // Serial.println("Client disconnected");
      // Remove the flag when a client disconnects
      g_is_wifi_client_connected = false;
      break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
      // Serial.println("Assigned IP address to client");
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
      // Serial.println("Received probe request");
      break;
    case SYSTEM_EVENT_GOT_IP6:
      // Serial.println("IPv6 is preferred");
      break;
    case SYSTEM_EVENT_ETH_START:
      // Serial.println("Ethernet started");
      break;
    case SYSTEM_EVENT_ETH_STOP:
      // Serial.println("Ethernet stopped");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      // Serial.println("Ethernet connected");
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      // Serial.println("Ethernet disconnected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      // Serial.println("Obtained IP address");
      break;
    default: break;
}}

void setup(void) {
  // Scenario 1 - firt power on (no WiFi config stored in EEPROM)
  // Start the softAP & restart when configuration is set

  // Scenario 2 - batteries changed (WiFi config stored in EEPROM)
  // Start the softAP for 1 minute to allow a device to connect and change config
  // If a device connects stop the timer
  // If the 1 minute timer elapses connect to the WiFi network

  // Scenario 3 - Device restarts to apply WiFi config
  // Connect to the WiFi network
  
  
  boolean network_connected = false;
  boolean init_soft_ap = false;
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  WiFi.onEvent(WiFiEvent);

  g_is_wifi_configured = get_wifi_credentials_from_eeprom(ssid, password);


  // Get the reset reason
  esp_reset_reason_t reset_reason = esp_reset_reason();
  if (reset_reason == ESP_RST_POWERON) {
    Serial.println("Power on");
    init_soft_ap = true;
  }
  
  if ((g_is_wifi_configured) && (init_soft_ap == false)) {
    // Connect to WiFi
    WiFi.begin(ssid, password);
    
    int connection_attempt = 0;
    while ((WiFi.status() != WL_CONNECTED) && (connection_attempt < 5)) {
      connection_attempt++;
      delay(1000);
      Serial.println("Connecting to WiFi..");
    }
    network_connected = (WiFi.status() == WL_CONNECTED);
  }

  if (network_connected == false) {
    Serial.println("Configuring access point...");
  
    bool softap_config_result = WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PSK);
    Serial.print("Soft AP Config Result: ");
    Serial.println(softap_config_result);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Server started");
  
  
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", htmlForm);
    });
    
    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for (int i = 0; i < params; i++)
      {
        AsyncWebParameter* p = request->getParam(i);
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        if ((p->name() == "input-name") && p->value().length() > 3) {
          Serial.println("Setting ESSID");
          strcpy(ssid, p->value().c_str());
          Serial.println(ssid);
        }
        if ((p->name() == "input-password") && (p->value().length() > 3)) {
          Serial.println("Setting PSK");
          strcpy(password, p->value().c_str());
          Serial.println(password);
        }
      }
      
      save_wifi_credentials_to_eeprom(ssid, password, 0, 96);
      request->send(200, "text/html", "<h1>Restarting to apply settings</h1>");
      ESP.restart();
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(404, "text/html", "<h1>404 Page Not Found</h1>");
    });

    // Configure mDNS
    boolean dns_configuration_result = MDNS.begin("esp32");
    
    if (!dns_configuration_result) {
        Serial.println("Error setting up MDNS responder!");
    } else {
      Serial.println("mDNS responder started");
    }
  
    server.begin();                            // Actually start the server
    Serial.println("HTTP server started");
  } else {
    Serial.println("Connected");
    Serial.println(WiFi.localIP());
  }
}

void loop(void) {
  unsigned long millis_now = millis();
  unsigned long millis_now_plus_60_seconds = millis_now + (60 * 1000);
  
  while(millis() < millis_now_plus_60_seconds) {
    if (g_is_wifi_client_connected) {
      break;
    }
    delay(1);
  }
  
  if ((g_is_wifi_client_connected == false) && g_is_wifi_configured) {
    Serial.println("Restart");
    ESP.restart();
  }
}
