#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include "FS.h"
#include <string.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define MAX_STRING_LEN  32

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8000);
const char *ssid = "*****";
const char *password = "*****";

unsigned long previousMillis = 0;        // will store last time LED was updated

// loop delay in milli seconds
const long interval = 2000;


// Function to return a substring defined by a delimiter at an index
char* subStr (char* str, char *delim, int index) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;

  // Since strtok consumes the first arg, make a copy
  strcpy(copy, str);

  for (i = 1, act = copy; i <= index; i++, act = NULL) {
     //Serial1.print(".");
     sub = strtok_r(act, delim, &ptr);
     if (sub == NULL) break;
  }
  return sub;

}
boolean saveData = false;
boolean broadcast = false;   
long duration, serverHeight; 
int clientHeight,ZeroOffset;
char str[8];
unsigned long currESPSecs, currTime,timestamp, zeroTime;

static const char PROGMEM INDEX_HTML[] = R"rawliteral( )rawliteral";




void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial1.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial1.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial1.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      //itoa( cm, str, 10 );
      //  webSocket.sendTXT(num, str, strlen(str));
      }
      break;
    case WStype_TEXT:
    {
      //Serial1.printf("[%u] get Text: %s\r\n", num, payload);

      //Payload will be in the form of 3 alphapets and 3 digits
      //DATA-105 means it is Data and value is 105
      //COMMAND-ZERO means Command ZERO
      //TIME-12345678 means epoch timestamp in seconds
      
      char *split1,*split2 ;
     char *mystring = (char *)payload;
      split1=subStr(mystring, "-", 1);
      split2=subStr(mystring, "-", 2);
      
      if (strcmp(split1,"COMMAND") == 0)
        {
          //Serial1.println("Received Command");
          //Serial1.println(split2);
          if (strcmp(split2,"ZERO") == 0){
           // Serial1.println("Zero command received");
            if (!clientHeight){
              Serial1.println("No client data");
              } else {
            ZeroOffset=serverHeight-clientHeight;
            zeroTime=timestamp+((millis()/1000)-currESPSecs);
            //Serial1.println(currTime);
            File f = SPIFFS.open("/Zero.txt", "w");
            f.print("Time:");
            f.print(zeroTime);
            f.print("\n");
            f.print("ZeroOffset:");
            f.print(ZeroOffset);
            f.print("\n");
            f.close();
            //Reset the currESPsecs to current millis at the zerotime
            //So the data save time will be zero time + secs elapsed since zero time
            currESPSecs = millis()/1000;
            
            }
          } else if (strcmp(split2,"STARTSAVE") == 0){
            
            if (!saveData){
              saveData=true;
            File f = SPIFFS.open("/Data.txt", "w");
            f.close();
            //Create a file which keeps log of current saving process
            File s = SPIFFS.open("/Save.txt", "w");
            s.print("Y");
            s.close();
            }
            
            } 

            else if (strcmp(split2,"STOPSAVE") == 0) 
            {
              if (saveData){saveData=false;
                File s = SPIFFS.open("/Save.txt", "w");
                s.print("N");
                s.close();
              }
              }

             
            else if (strcmp(split2,"RUSAVING") == 0) 
            {
              if (saveData){
                webSocket.sendTXT(num, "Y");
                } else {
                  
                webSocket.sendTXT(num, "N");  
                  }
              }

        
        } 
      else if (strcmp(split1,"DATA") == 0)
        {
          //Serial1.println("Received Data");
          //Serial1.println(split2);
          clientHeight=atoi(split2);
          if (saveData){
          //currTime=timestamp+((millis()/1000)-currESPSecs);
          currTime=((millis()/1000)-currESPSecs);
          File f = SPIFFS.open("/Data.txt", "a");
          f.print(currTime);
          f.print(":");
          f.print("C:");
          f.print(clientHeight);
          f.print("\n");
          f.close();
          }
          if(broadcast){
           webSocket.broadcastTXT(split2, strlen(split2));
            } else {
              //If not broadcast - send to browser client num =1
              webSocket.sendTXT(1,split2, strlen(split2));
              
              }
          
        } 
      else if (strcmp(split1,"TIME") == 0)
        {
          //Serial1.println("Received TimeStamp");
          //Serial1.println(split2);
          timestamp=atol(split2);
          //currTime=split2;
          currESPSecs = millis()/1000;
          //Serial1.println(timestamp);
          //Serial1.println(currESPSecs);
        }
      else 
      {
          Serial1.printf("Unknown-");
          Serial1.printf("[%u] get Text: %s\r\n", num, payload);
          // send data to all connected clients
          //webSocket.broadcastTXT(payload, length);
      }
    }
    // clientHeight=atoi((const char *)payload);
     // Serial1.println((const char *)payload);
     // itoa( cm, str, 10 );
      // webSocket.sendTXT(0, str, strlen(str));
      break;
    case WStype_BIN:
      Serial1.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial1.printf("Invalid WStype [%d]\r\n", type);
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

/*

//String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)
//String getContentType(String filename) { // convert the file extension to the MIME type
//  if (filename.endsWith(".html")) return "text/html";
//  else if (filename.endsWith(".css")) return "text/css";
//  else if (filename.endsWith(".js")) return "application/javascript";
//  else if (filename.endsWith(".ico")) return "image/x-icon";
//  return "text/plain";
//}


bool handleFileRead(String path) { // send the right file to the client (if it exists)
  //Serial1.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  //String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    //size_t sent = server.streamFile(file, contentType); // And send it to the client
    size_t sent = server.streamFile(file, "text/plain"); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  //Serial1.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}

*/

void setup()
{ 


  Serial1.begin(115200);
  delay(10);
  //Serial1.setDebugOutput(true);

  Serial1.println();
  Serial1.println();
  Serial1.println();

  for(uint8_t t = 4; t > 0; t--) {
    Serial1.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial1.flush();
    delay(1000);
  }

  Serial1.print("Configuring access point...");
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial1.print("AP IP address: ");
 Serial1.println(myIP);
  

  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
//  server.onNotFound([]() {                              // If the client requests any URI
//    if (!handleFileRead(server.uri()))                  // send it if it exists
//      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
//  });

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);


/* ************SPIFFS********************* */
//  if (SPIFFS.begin()){Serial1.println("file system mounted");};
//
//  //Open the "Save.txt" file and check if we were saving before the reset happened
//  File q = SPIFFS.open("/Save.txt", "r");
//  if (q.find("Y")){saveData=true;}
//  q.close();

 /*********************************************/
 
 /* ************OTA********************* */
    // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");
   
   ArduinoOTA.onStart([]() {
    Serial1.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial1.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial1.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial1.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial1.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial1.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial1.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial1.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial1.println("End Failed");
  });
  ArduinoOTA.begin();
/****************************************************/  
  }



void loop()
{
  webSocket.loop();
  server.handleClient();
  ArduinoOTA.handle();
  unsigned long currentMillis = millis();
 delay(10);
  if(currentMillis - previousMillis >= interval) {
   
    previousMillis = currentMillis;  
// Timer has run out
  }
// Yield to WiFi  
//delay(5);
}
