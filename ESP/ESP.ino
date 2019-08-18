#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <string.h>
#define MAX_STRING_LEN  32



const char *ssid = "********";
const char *password = "********";



ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8000);


int trigPin = 4;    
int echoPin = 5;    
long duration, cm, inches; 
char measurement[10] = {'M'};
char calibration[10] = {'C'};
static const char PROGMEM INDEX_HTML[] = R"rawliteral()rawliteral";





void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
switch(type) {
case WStype_DISCONNECTED:
Serial.printf("[%u] Disconnected!\r\n", num);
break;
case WStype_CONNECTED:
{
IPAddress ip = webSocket.remoteIP(num);
Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
//itoa( cm, str, 10 );
//  webSocket.sendTXT(num, str, strlen(str));
}
break;
case WStype_TEXT:
{
Serial.printf("[%u] get Text: %s\r\n", num, payload);
//Payload will be - FOR REV STOP

char *mystring = (char *)payload;

if (strcmp(mystring,"ZERO") == 0)
{
doCalibrate();
//webSocket.broadcastTXT(payload, length);
}
else if (strcmp(mystring,"MEASURE") == 0) 
{
doMeasure();
//webSocket.broadcastTXT(payload, length);
} 

}
break;
case WStype_BIN:
Serial.printf("[%u] get binary length: %u\r\n", num, length);
hexdump(payload, length);

// echo data back to browser
webSocket.sendBIN(num, payload, length);
break;
default:
Serial.printf("Invalid WStype [%d]\r\n", type);
break;
}
}

void handleRoot()
{
server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
String message = "File Not Found\n\n";
message += "URI: ";
message += server.uri();
message += "\nMethod: ";
message += (server.method() == HTTP_GET)?"GET":"POST";
message += "\nArguments: ";
message += server.args();
message += "\n";
for (uint8_t i=0; i<server.args(); i++){
message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
}
server.send(404, "text/plain", message);
}


void doCalibrate()
{
Serial.println("Calibrating");
hcsr04();
itoa( duration, calibration+1, 10 );
webSocket.broadcastTXT(calibration,10);

}

void doMeasure()
{
Serial.println("Measuring");
hcsr04();
itoa( duration, measurement+1, 10 );
webSocket.broadcastTXT(measurement,10);


}
void hcsr04()
{
//HR-SC04 
// The sensor is triggered by a HIGH pulse of 10 or more microseconds.
// Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
digitalWrite(trigPin, LOW);
delayMicroseconds(5);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

// Read the signal from the sensor: a HIGH pulse whose
// duration is the time (in microseconds) from the sending
// of the ping to the reception of its echo off of an object.
//pinMode(echoPin, INPUT);
duration = pulseIn(echoPin, HIGH);
//delay(ms) pauses the sketch for a given number of milliseconds and allows WiFi and TCP/IP tasks to run. 
//delayMicroseconds function, on the other hand, does not yield to other tasks, so using it for delays more than 20 milliseconds is not recommended
delayMicroseconds(10000);
// convert the time into a distance
//cm = (duration/2) / 29.1;
//inches = (duration/2) / 74; 

//Serial.print(inches);
//Serial.print("in, ");
//  Serial.print(cm);
//  Serial.print("cm");
//  Serial.println();
}
void setup()
{ 


Serial.begin(115200);
delay(10);

// HR-SC04
//Define inputs and outputs
pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT);

Serial.println();
Serial.println();
Serial.println();

for(uint8_t t = 4; t > 0; t--) {
Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
Serial.flush();
delay(1000);
}

WiFiMulti.addAP(ssid, password);

while(WiFiMulti.run() != WL_CONNECTED) {
Serial.print(".");
delay(100);
}

Serial.println("");
Serial.print("Connected to ");
Serial.println(ssid);
Serial.print("IP address: ");
Serial.println(WiFi.localIP());


Serial.print("Connect to http://");
Serial.println(WiFi.localIP());

server.on("/", handleRoot);
server.onNotFound(handleNotFound);

server.begin();

webSocket.begin();
webSocket.onEvent(webSocketEvent);



}

void loop()
{
webSocket.loop();
server.handleClient();
}
