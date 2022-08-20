// include library to read and write from flash memory
#include <EEPROM.h>

#include "wifi_config.h"

bool get_wifi_credentials_from_eeprom(char* ssid, char* password) {
  EEPROM.begin(EEPROM_SIZE);
  get_wifi_ssid_from_eeprom(ssid);
  get_wifi_psk_from_eeprom(password);
  
  const bool configured = (bool)EEPROM.read(EEPROM_SIZE - 1);
  EEPROM.end();

  return configured;
}

void get_wifi_ssid_from_eeprom(char* ssid) {
  // SSID
  // Max length of SSID is 32 characters
  // These SSIDs can be zero to 32 octets (32 bytes) long
  // https://en.wikipedia.org/wiki/Service_set_(802.11_network)
  for (int i = 0; i < 32; i++) {
    ssid[i] = (char)EEPROM.read(i);
  }
  return;  
}

void get_wifi_psk_from_eeprom(char* password) {
  // Password
  // WPA personal - a string of 64 hexadecimal digits, or as a passphrase of 8 to 63 printable ASCII characters
  // https://en.wikipedia.org/wiki/Wi-Fi_Protected_Access
  for (int i = 0; i < 63; i++) {
    password[i] = (char)EEPROM.read((i + 32));
  }
  return;
}

int save_wifi_credentials_to_eeprom(char* ssid, char* password, int start_address, int end_address) {
  const int ssid_length = 32;
  const int password_length = 63;
  const int wifi_credentials_length = (ssid_length + password_length) + 1; // (ssid + password + configured flag)

  const int ssid_offset = start_address;
  const int password_offset = start_address + ssid_length;
  EEPROM.begin(EEPROM_SIZE);
  
  // Start address can't be negative
  if (start_address < 0) {
    return 0;
  }
  // Check end address isn't less than start address + bytes required to store wifi credentials
  if (end_address < (start_address + wifi_credentials_length)) {
    return 0;
  }
  
  // SSID
  // Max length of SSID is 32 characters
  // These SSIDs can be zero to 32 octets (32 bytes) long
  // https://en.wikipedia.org/wiki/Service_set_(802.11_network)
  for (int i = start_address; i < ssid_length; i++) {
    EEPROM.write((i + ssid_offset), ssid[i]);
  }

  // Password
  // WPA personal - a string of 64 hexadecimal digits, or as a passphrase of 8 to 63 printable ASCII characters
  // https://en.wikipedia.org/wiki/Wi-Fi_Protected_Access
  for (int i = 0; i < password_length; i++) {
    EEPROM.write((password_offset + i), password[i]);
  }

  // Set configured to true
  EEPROM.write((start_address + wifi_credentials_length), 1);

  EEPROM.commit();
  EEPROM.end();
  return 1;
}