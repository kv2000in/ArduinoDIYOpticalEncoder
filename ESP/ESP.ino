#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include "FS.h"
#include <string.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti WiFiMulti;

#define MAX_STRING_LEN  32

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8000);
const char *ssid = "**";
const char *password = "**";

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

static const char PROGMEM INDEX_HTML[] = R"rawliteral(<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=2.7">

  <meta  content="text/html; charset=utf-8">
<style>
  * {
    box-sizing: border-box;
}
  
  [class*="col-"] {
    float: left;
    padding: 15px;
}
/* For mobile phones: */
[class*="col-"] {
    width: 100%;
}
@media only screen and (min-width: 1024px) {
    /* For desktop: */
    .col-1 {width: 8.33%;}
    .col-2 {width: 16.66%;}
    .col-3 {width: 25%;}
    .col-4 {width: 33.33%;}
    .col-5 {width: 41.66%;}
    .col-6 {width: 50%;}
    .col-7 {width: 58.33%;}
    .col-8 {width: 66.66%;}
    .col-9 {width: 75%;}
    .col-10 {width: 83.33%;}
    .col-11 {width: 91.66%;}
    .col-12 {width: 100%;}
}
  .switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}
.svrlight{
  background-color: red;
  }
.switch input {display:none;}
.pwrlight{
  position: relative;
  height: 26px;
  width: 26px;
  right: 20px;
  bottom: 4px;
  border-radius: 10%;
  }
.pwrlight:active{
  background-color: red;
  border: none;
  }
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}
.buttonpressedeffect {
  display: inline-block;
  padding: 15px 25px;
  font-size: 24px;
  cursor: pointer;
  text-align: center;
  text-decoration: none;
  outline: none;
  color: #fff;
  background-color: #4CAF50;
  border: none;
  border-radius: 15px;
  box-shadow: 0 9px #999;
}

.buttonpressedeffect:hover {background-color: #3e8e41}

.buttonpressedeffect:active {
  background-color: #3e8e41;
  box-shadow: 0 5px #666;
  transform: translateX(4px);
 }
 
 .buttonhover {
  display: inline-block;
  border-radius: 4px;
  background-color: #f4511e;
  border: none;
  color: #FFFFFF;
  text-align: center;
  font-size: 28px;
  padding: 20px;
  width: 200px;
  transition: all 0.5s;
  cursor: pointer;
  margin: 5px;
}

.buttonhover span {
  cursor: pointer;
  display: inline-block;
  position: relative;
  transition: 0.5s;
}

.button span:after {
  content: '\00bb';
  position: absolute;
  opacity: 0;
  top: 0;
  right: -20px;
  transition: 0.5s;
}

.buttonhover:hover span {
  padding-right: 25px;
}

.button:hover span:after {
  opacity: 1;
  right: 0;
}

</style>
<script language="javascript" type="text/javascript">

 var boolConnected=false;
  function doConnect()
  {
      if (!(boolConnected)){
      /*websocket = new WebSocket(document.myform.url.value);*/
/*
         websocket = new WebSocket('ws://192.168.1.106:8000/');
*/

         
    websocket = new WebSocket('ws://' + window.location.hostname + ':8000/'); 
    boolConnected=true;
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
  }

  }
  function onOpen(evt)
  {
    console.log("connected\n");

  }

  function onClose(evt)
  {
    console.log("disconnected\n");
      /* if server disconnected - change the color to red*/

      boolConnected=false;
  }

  function onMessage(evt)
  {

      
     /* Data returned from ESP will be in the form of ON-1 OFF-1 ON-2 ON-5 OFF-2 OFF-5 CHKA-1010
      CHKA-1010 4 digits correspond to gpio reads of pin 1,2,5,6
    evt.data.slice(-1) = "1" or "2" or "5" or "6"
    evt.data.slice(0,-2)="ON" or "OFF" 
      evt.data.slice(0,-5)="CHKA" 
      
      Data returned from ESP will be in the form of FOR REV STOP
      
      */
      
      
    console.log("response: " + evt.data + '\n');
      document.getElementById("mytxtarea").innerHTML = evt.data;
        if(evt.data=="FOR"){ /* Going forward*/
        document.getElementById("pwrlightFOR").style.backgroundColor="red";
        document.getElementById("sw_FOR").checked=true;
        document.getElementById("sw_REV").disabled=true;
        };
    if(evt.data=="REV"){ /* Going Reverse */
        document.getElementById("pwrlightREV").style.backgroundColor="red";
        document.getElementById("sw_REV").checked=true;
        document.getElementById("sw_FOR").disabled=true;
        };
    if(evt.data=="STOP"){ /* Stopped*/
        document.getElementById("pwrlightFOR").style.backgroundColor="black";
        document.getElementById("pwrlightREV").style.backgroundColor="black";
        document.getElementById("sw_FOR").disabled=false;
        document.getElementById("sw_REV").disabled=false;
        document.getElementById("sw_FOR").checked=false;
        document.getElementById("sw_REV").checked=false;
        };  

    
    
  
  }

  function onError(evt)
  {

  console.log('error: ' + evt.data + '\n');
  websocket.close();


  }

  function doSend(message)
  {
    console.log("sent: " + message + '\n');

    websocket.send(message);
  }

function closeWS(){
    
   /* Probable bug in arduino websocket - hangs if not closed properly, specially by a phone browser entering a powersaving mode*/
    websocket.close();
    boolConnected=false;
}

/*    On android - when page loads - focus event isn't fired so websocket doesn't connect*/
   
   window.addEventListener("focus",doConnect, false);
   
    
 window.addEventListener("blur",closeWS, false);
 window.addEventListener('load', function() {
    foo(true); 
     /*After page loading blur doesn't fire until focus has fired at least once*/
     
    /* window.focus();*/
},{once:true}, false);

/*window.addEventListener('blur', function() {
    foo(false);
}, {once:true}, false); */


function foo(bool) {
    if (bool){
doConnect();
    } else {
        
      
 /*   Probable bug in arduino websocket - hangs if not closed properly, specially by a phone browser entering a powersaving mode
 */       websocket.close();    
    }
}
    
</script>
<script type="text/javascript">


function queryServer1(direction)
{

  var payload;
  if (document.getElementById("sw_"+direction).checked==true){
        payload=direction;}
        else {
        
        payload="STOP";
        };
 

  doSend(payload);
 
};
function returntoZero(){
  var myreadingintxtarea=document.getElementById("mytxtarea").innerHTML.split("-");
 if(myreadingintxtarea) //check to see if it contains something = not blank = not UNDEFINED
    {
    
    if (myreadingintxtarea [myreadingintxtarea.length-1] =="0") //Already at zero - do nothing. Log
     {
         console.log("At Zero");
         
     }
    else
        {
            if (myreadingintxtarea.length==3) //length will be 3 if it is 0--10000 , meaning -it is at negative position. Otherwise length will be 2.
            {
                doSend("<F-"+myreadingintxtarea[2]+">");
            }
            else if (myreadingintxtarea.length==2)
            {
                doSend("<R-"+myreadingintxtarea[1]+">"); 
            }
        }
    }
 };
    
</script>
<title>Serial COM </title></head>
<body>


<div class = "col-6">
  <textarea id="mytxtarea">
    
    </textarea>

<input id = "serialSend">
    <button id = "btnserialSend" onclick='doSend(document.getElementById("serialSend").value)'>SEND</button>
    <button id = "btnserialSendSTOP" onclick='doSend("<S-1>")'>STOP</button>
    <button id = "btnserialSendFOR" onclick='doSend("<F-10000>")'>F-10000</button>
    <button id = "btnserialSendREV" onclick='doSend("<R-10000>")'>R-10000</button>
    <button id = "btnserialSendRET2ZERO" onclick='returntoZero()'>Zero</button>
    
</div>

</body>


</html>)rawliteral";




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

          } else if (strcmp(split2,"STARTSAVE") == 0){
            

            } 

            else if (strcmp(split2,"STOPSAVE") == 0) 
            {

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

          clientHeight=atoi(split2);

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
        //Send commands coming via websocket to atmega
        Serial.print(mystring);
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
/*******************Serial Read Functions ************************/
//Serial Read stuff
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;
char DIR[2] = {'X','Y'};
//char DIR = 'Z';
//long VALUE = 0;
char VALUE[2]={'W','Z'};
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
 // if (Serial.available() > 0) {
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                //Broadcast received serial comm from Atmega - back to websocket
                webSocket.broadcastTXT(receivedChars,ndx);
                ndx = 0;
                parseData();
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        Serial1.print("This just in ... ");
        Serial1.println(receivedChars);
        Serial1.print("DIRECTION=");
        Serial1.println(DIR);
        Serial1.print("VALUE=");
        Serial1.println(VALUE);
        newData = false;
    }
}

void parseData() {


    // split the data into its parts
    
  char * strtokIndx; // this is used by strtok() as an index
  
  strtokIndx = strtok(receivedChars,"-");      // get the first part - the string
  strcpy(DIR, strtokIndx); // copy it to DIR
  //strcpy(DIR,0);
  //strtokIndx = strtok(NULL, "-"); // this continues where the previous call left off
  //VALUE = atoi(strtokIndx); // convert this part to an integer
  //VALUE = atol(strtokIndx+2);   
  strcpy(VALUE, strtokIndx+3); // copy it to DIR
 }
/*******************Serial Read Functions ************************/

void setup()
{ 


  Serial1.begin(115200);
  delay(10);
  Serial.begin(57600);
  //Serial1.setDebugOutput(true);

  Serial1.println();
  Serial1.println();
  Serial1.println();

  for(uint8_t t = 4; t > 0; t--) {
    Serial1.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial1.flush();
    delay(1000);
   Serial.print("<f-10000>");
  }
/***************** AP mode*******************/
//  Serial1.print("Configuring access point...");
//  WiFi.softAP(ssid, password);
//
//  IPAddress myIP = WiFi.softAPIP();
//  Serial1.print("AP IP address: ");
// Serial1.println(myIP);
  
/***********************************************/

/*****************Client Mode******************/
  WiFiMulti.addAP(ssid, password);

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial1.print(".");
    delay(100);
  }

  Serial1.println("");
  Serial1.print("Connected to ");
  Serial1.println(ssid);
  Serial1.print("IP address: ");
  Serial1.println(WiFi.localIP());
/**********************************************/
  
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
  recvWithStartEndMarkers();
//  unsigned long currentMillis = millis();
// delay(10);
//  if(currentMillis - previousMillis >= interval) {
//   Serial.print("<r-1000>");
//    previousMillis = currentMillis; 
//  
//// Timer has run out
//  }
showNewData();
//   if (newData) {
//    Serial1.print(DIR[0],VALUE);
//   newData=false;
//   }
// Yield to WiFi  
//delay(5);
}
