#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include "index.h"
//
ESP8266WiFiMulti WifiMulti;
//
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
//
#include <Servo.h>
Servo sx,sy,sz,sg;
int sxo=90;
//
void servo(Servo s,int p, int v) {
//  s.attach(p);
// analogWrite(p, 25+(85*v));  /* Write duty cycle to pin */
// delay(15);
if (v==0) v=1;
  int d=8+(abs(sxo-v)*2);
  Serial.printf("Origem: %u \t Angle: %u \t Delay: %u\n",sxo,v,d);
  for (int t=0;t<3;t++){
  s.write(v);
  delay(d);
  }
  sxo=v;
//  s.writeMicroseconds(500+(v*((2400-500)/180)));
//  s.detach();
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            webSocket.sendTXT(num, "Connected");
        }   break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            uint32_t rgb = (uint32_t) strtoul((const char *) &payload[1], NULL, 16);
            if(payload[0] == 'X') {
                int val = map(((rgb >> 16) & 0xFF), 0, 180, 0, 180);
                servo(sx,5,val);
                Serial.printf("Servo X:%u\n", val);
            } else  
            if(payload[0] == 'Y') {
                int val = map(((rgb >> 16) & 0xFF), 0, 180, 0, 180);
                servo(sy,14,val);
                Serial.printf("Servo Y:%u\n", val);
            } else 
            if(payload[0] == 'Z') {
                int val = map(((rgb >> 16) & 0xFF), 0, 180, 0, 180);
                servo(sz,12,val);
                Serial.printf("Servo Z:%u\n", val);
            } else 
            if(payload[0] == 'G') {
                int val = map(((rgb >> 16) & 0xFF), 0, 180, 0, 180);
                servo(sg,13,val);
                Serial.printf("Servo Z:%u\n", val);
            } else
            if(payload[0] == 'R') {
                Serial.printf("Servo Z position(%u)\n", sz.read());
            } else
            {
                int aa = ((rgb >>  0) & 0xFF);
                int bb = ((rgb >>  8) & 0xFF);
                int cc = ((rgb >> 16) & 0xFF);
                int dd = ((rgb >> 24) & 0xFF);
                Serial.printf("Special rgb(%u) a(%u) b(%u) c(%u) d(%u)\n", rgb, aa, bb, cc, dd);
                if(payload[0] == 'a') for(int x=aa;x<=bb;x+=cc){sz.write(x);delay(dd);}
                if(payload[0] == 'A') for(int x=bb;x>=aa;x-=cc){sz.write(x);delay(dd);}
            }
            break;
    }
}
//
void wifiSetup() {
  WifiMulti.addAP("SSID-1","PASS-1");
  WifiMulti.addAP("SSID-2","PASS-2");
  WifiMulti.addAP("SSID-3","PASS-3");
  Serial.println("Connecting ...");
  for (int i = 0; WifiMulti.run() != WL_CONNECTED; delay(500)) { 
    Serial.printf("%u ",++i);
  }
  Serial.println('\n');
  Serial.printf("Connected to %s\n",WiFi.SSID().c_str()); // Tell us what network we're connected to
  Serial.printf("IP address:\t %s\n",WiFi.localIP().toString().c_str()); // Send the IP address of the ESP8266 to the computer
  if (!MDNS.begin("NodeMcu")) {             // Start the mDNS responder for esp8266.local
    Serial.println("Error setting up MDNS responder!");
  }
  Serial.println("mDNS responder started");
}
//
void setup() {
  Serial.begin(115200);
  sx.attach(5,450,2450);//13 wemos
//  sx.attach(5);//13 wemos
  sy.attach(14,1000,2000);//12
  sz.attach(12,1000,2000);//14
  sg.attach(13,1000,2000);//4
  for(int i=80;i<100;i++){
  servo(sx,5,i);
  }   servo(sx,5,90);
  Serial.printf("Servo x position(%u)\n", sx.read());
  Serial.printf("Servo y position(%u)\n", sy.read());
  Serial.printf("Servo z position(%u)\n", sz.read());
  Serial.printf("Servo g position(%u)\n", sg.read());
  wifiSetup();
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // handle index
  server.on("/", []() {
     server.send(200, "text/html", MAIN_page); // index.h
     // server.send(200, "text/html", "<html><head><script> var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']); connection.onopen = function () {      connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {      console.log('WebSocket Error ', error); }; connection.onmessage = function (e) {      console.log('Server: ', e.data);    }; function sendServo(id,value) {      var s = id + parseInt(value).toString(16) +'0000';      console.log('SERVO: ' + s );   connection.send(s);   } </script> </head><body><center>ESP8266<br>WebSocket Arm Control SG90<br> <input id=\"X\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendServo(this.id,this.value);\"><br> <input id=\"Y\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendServo(this.id,this.value);\"><br> <input id=\"Z\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendServo(this.id,this.value);\"><br> <input id=\"G\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendServo(this.id,this.value);\"><br> </center></body></html>");
  });
  server.begin();
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws"  , "tcp", 81);
  //
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
//
unsigned long t;
unsigned long last_10sec = 0;
unsigned int counter = 0;
//
void loop() {
    t = millis();
    webSocket.loop();
    server.handleClient();
    if((t - last_10sec) > 10 * 1000) {
        counter++;
        bool ping = (counter % 2);
        int i = webSocket.connectedClients(ping);
        Serial.printf("%d Connected websocket clients ping: %d\n", i, ping);
        last_10sec = millis();
    }
}
