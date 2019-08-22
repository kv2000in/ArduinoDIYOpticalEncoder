/* 
 ESP8266 CheckFlashConfig by Markus Sattler
 
 This sketch tests if the EEPROM settings of the IDE match to the Hardware
 
 */

void setup(void) {
    Serial.begin(115200);
    Serial1.begin(115200);
}

void loop() {
Serial1.println("I am Serial 1 on GPIO2 pin D4 on NodeMCU");
Serial.println("I am Serial on Tx");
    delay(5000);
}
