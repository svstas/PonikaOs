/*
#include <Adafruit_PN532.h>

time_t sc,osc;
int getNfc() {
osc = sc;
sc = now();
if (osc!=sc) sendEvent(sc);
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
 
 success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  // If the card is detected, print the UID
  if (success)
  {    
        sendEvent("FUCK");
    }
return 1;
}
*/

