#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include <ESPmDNS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "HTMLForm.h" // HTML Form
#include "wifi_config.h"

#define BAUD_RATE 115200

//#define NDEBUG

const char* SOFT_AP_SSID = "ESP32";
const char* SOFT_AP_PSK = "12345678"; // Must be 8 characters or more

char ssid[32] = "";
char password[63] = "";
static volatile boolean g_is_wifi_configured = false;
static volatile boolean g_is_wifi_client_connected = false;
static volatile boolean g_is_wifi_connected = false;

AsyncWebServer server(80);

void WiFiEvent(WiFiEvent_t event) {
  #ifndef NDEBUG
  Serial.printf("[WiFi-event] event: %d\n", event);
  #endif

  switch (event) {
    case SYSTEM_EVENT_WIFI_READY: 
      #ifndef NDEBUG
      Serial.println("WiFi interface ready");
      #endif
      break;
    case SYSTEM_EVENT_SCAN_DONE:
      #ifndef NDEBUG
      Serial.println("Completed scan for access points");
      #endif
      break;
    case SYSTEM_EVENT_STA_START:
      #ifndef NDEBUG
      Serial.println("WiFi client started");
      #endif
      break;
    case SYSTEM_EVENT_STA_STOP:
      #ifndef NDEBUG
      Serial.println("WiFi clients stopped");
      #endif
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      #ifndef NDEBUG
      Serial.println("Connected to access point");
      #endif
      g_is_wifi_connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      #ifndef NDEBUG
      Serial.println("Disconnected from WiFi access point");
      #endif
      g_is_wifi_connected = false;
      break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
      #ifndef NDEBUG
      Serial.println("Authentication mode of access point has changed");
      #endif
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      #ifndef NDEBUG
      Serial.print("Obtained IP address: ");
      Serial.println(WiFi.localIP());
      #endif
      break;
    case SYSTEM_EVENT_STA_LOST_IP:
      #ifndef NDEBUG
      Serial.println("Lost IP address and IP address is reset to 0");
      #endif
      break;
    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
      #ifndef NDEBUG
      Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      #endif
      break;
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:
      #ifndef NDEBUG
      Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
      #endif
      break;
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
      #ifndef NDEBUG
      Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
      #endif
      break;
    case SYSTEM_EVENT_STA_WPS_ER_PIN:
      #ifndef NDEBUG
      Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
      #endif
      break;
    case SYSTEM_EVENT_AP_START:
      #ifndef NDEBUG
      Serial.println("WiFi access point started");
      #endif
      break;
    case SYSTEM_EVENT_AP_STOP:
      #ifndef NDEBUG
      Serial.println("WiFi access point  stopped");
      #endif
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      #ifndef NDEBUG
      Serial.println("Client connected");
      #endif
      // Set the flag when a client connects
      g_is_wifi_client_connected = true;
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      #ifndef NDEBUG
      Serial.println("Client disconnected");
      #endif
      
      // Remove the flag when a client disconnects
      g_is_wifi_client_connected = false;
      break;
    case SYSTEM_EVENT_AP_STAIPASSIGNED:
      #ifndef NDEBUG
      Serial.println("Assigned IP address to client");
      #endif
      break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
       #ifndef NDEBUG
      Serial.println("Received probe request");
      #endif
      break;
    case SYSTEM_EVENT_GOT_IP6:
      #ifndef NDEBUG
      Serial.println("IPv6 is preferred");
      #endif
      break;
    case SYSTEM_EVENT_ETH_START:
      #ifndef NDEBUG
      Serial.println("Ethernet started");
      #endif
      break;
    case SYSTEM_EVENT_ETH_STOP:
      #ifndef NDEBUG
      Serial.println("Ethernet stopped");
      #endif
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      #ifndef NDEBUG
      Serial.println("Ethernet connected");
      #endif
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      #ifndef NDEBUG
      Serial.println("Ethernet disconnected");
      #endif
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      #ifndef NDEBUG
      Serial.println("Obtained IP address");
      #endif
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

  // Scenario 4 - WiFi network is not in range (WiFi config stored in EEPROM)
  // Same behaviour as in scenario 2
  // Start the softAP for 1 minute to allow a device to connect and change config
  // If a device connects stop the timer
  // If the 1 minute timer elapses connect to the WiFi network

  // Scenario 5 - WiFi network name or password has been changed on the router / AP (WiFi config stored in EEPROM)
  // Same behaviour as in scenario 2
  // Start the softAP for 1 minute to allow a device to connect and change config
  // If a device connects stop the timer
  // If the 1 minute timer elapses connect to the WiFi network
  
  
  boolean network_connected = false;
  boolean init_soft_ap = false;
  #ifndef NDEBUG
  Serial.begin(BAUD_RATE);         // Start the Serial communication to send messages to the computer
  #endif
  
  delay(10);
  WiFi.onEvent(WiFiEvent);

  g_is_wifi_configured = get_wifi_credentials_from_eeprom(ssid, password);


  // Get the reset reason
  esp_reset_reason_t reset_reason = esp_reset_reason();
  if (reset_reason == ESP_RST_POWERON) {
    #ifndef NDEBUG
    Serial.println("Power on");
    #endif
    init_soft_ap = true;
    g_is_wifi_configured = false;
  }
  
  if ((g_is_wifi_configured) && (init_soft_ap == false)) {
    // Connect to WiFi
    if (strlen(password) != 0) {
      #ifndef NDEBUG
      Serial.println("WiFi password detected in saved config");
      #endif
      WiFi.begin(ssid, password);
    } else {
      #ifndef NDEBUG
      Serial.println("Attempting to connect to OPEN WiFi network");
      #endif
      WiFi.begin(ssid);
    }
    
    int connection_attempt = 0;
    while ((WiFi.status() != WL_CONNECTED) && (connection_attempt < 5)) {
      connection_attempt++;
      delay(1000);
      #ifndef NDEBUG
      Serial.println("Connecting to WiFi..");
      #endif
    }
    network_connected = (WiFi.status() == WL_CONNECTED);
    g_is_wifi_connected = network_connected;
  }

  if (network_connected == false) {
    #ifndef NDEBUG
    Serial.println("Configuring access point...");
    #endif
  
    bool softap_config_result = WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PSK);
    
    #ifndef NDEBUG
    Serial.print("Soft AP Config Result: ");
    Serial.println(softap_config_result);
    #endif
    
    IPAddress myIP = WiFi.softAPIP();
    #ifndef NDEBUG
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.println("Server started");
    #endif
  
  
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", htmlForm);
    });
    
    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for (int i = 0; i < params; i++)
      {
        AsyncWebParameter* p = request->getParam(i);
        #ifndef NDEBUG
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        #endif
        if ((p->name() == "input-name") && p->value().length() > 3) {
          strcpy(ssid, p->value().c_str());
          
          #ifndef NDEBUG
          Serial.println("Setting ESSID");
          Serial.println(ssid);
          #endif
        }

        if ((p->name() == "input-wifi-security") && p->value().length() > 3) {
          #ifndef NDEBUG
          Serial.println(p->value().c_str());
          #endif
          // If WiFi security type is "open" then set the password to an empty string
          if (strcmp(p->value().c_str(), "open") == 0) {
            #ifndef NDEBUG
            Serial.println("Removing password - auth mode is open");
            #endif
            strcpy(password, "");
          }
        }

        if ((p->name() == "input-password") && (p->value().length() > 3)) {
          strcpy(password, p->value().c_str());
          #ifndef NDEBUG
          Serial.println("Setting PSK");
          Serial.println(password);
          #endif
        }
      }
      
      save_wifi_credentials_to_eeprom(ssid, password, 0, 96);
      request->send(200, "text/html", "<h1>Restarting to apply settings</h1>");
      // Bug - intermittent web page not available (device restarts before web page can be sent to client)
      g_is_wifi_configured = true;
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
      request->send(404, "text/html", "<h1>404 Page Not Found</h1>");
    });

    // Configure mDNS
    boolean dns_configuration_result = MDNS.begin("esp32");
    
    if (!dns_configuration_result) {
        #ifndef NDEBUG
        Serial.println("Error setting up MDNS responder!");
        #endif
    } else {
      #ifndef NDEBUG
      Serial.println("mDNS responder started");
      #endif
    }
  
    server.begin();                            // Actually start the server
    #ifndef NDEBUG
    Serial.println("HTTP server started");
    #endif
  } else {
    #ifndef NDEBUG
    Serial.println("Connected");
    Serial.println(WiFi.localIP());
    #endif
  } // END - if (network_connected == false)
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

  // Once configured, restart after 10 seconds
  if (g_is_wifi_client_connected && g_is_wifi_configured) {
      // Wait 10 seconds before restarting
      for (uint8_t i = 0; i < 10; i++) {
          delay(1000);
      }
      ESP.restart();
  }
  
  // BUG fix - reconnects to configured WiFi network every 60 seconds
  // Restart when wifi client is NOT connected, wifi IS configured in EEPROM and if NOT connected to a wifi network
  if ((g_is_wifi_client_connected == false) && g_is_wifi_configured && (g_is_wifi_connected == false)) {
    #ifndef NDEBUG
    Serial.println("Restart");
    #endif
   
    ESP.restart();
  }
}
