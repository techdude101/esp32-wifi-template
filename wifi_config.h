#ifndef _WIFI_CONFIG
#define _WIFI_CONFIG

#include <stdbool.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 97 // SSID = 32, PSK = 63, Flag for wifi config saved = 1

void get_wifi_ssid_from_eeprom(char* ssid);

void get_wifi_psk_from_eeprom(char* password);

bool get_wifi_credentials_from_eeprom(char* ssid, char* password);

int save_wifi_credentials_to_eeprom(char* ssid, char* password, int start_address, int end_address);

#endif