#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#include <SoftwareSerial.h>
SoftwareSerial softserial(4, 5); // A9 to ESP_TX, A8 to ESP_RX by default

char ssid[] = "";
char pass[] = "";

int status = WL_IDLE_STATUS;

int ledPin = 13;
int servoPin = 3;
int flashing_light = 0;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 

WiFiEspClient client;
WiFiEspUDP Udp;

unsigned int localPort = 8888;              // local port to listen on

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(servoPin, OUTPUT);

  Serial.begin(9600);
  softserial.begin(9600);
  
  WiFi.init(&softserial);

  while ( status != WL_CONNECTED) {
    
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  digitalWrite(ledPin, LOW);

  // if you get a connection, report back via serial:
  Udp.begin(localPort);
  Serial.print(" target port ");
  Serial.println(localPort);
 
}

long last_request = 0;

void request(){

  IPAddress server(192,168,1,92);
  char statusCode = client.connect(server, 3000);
  
  if(statusCode){
    last_request = millis();
    Serial.println("Connected to "+client.remoteIP());
    client.println("POST /chicken_coop/door/state");
    client.println("Host: ");
    client.println("Connection: close");
    client.println();
  }else{
    client.stop();
    Serial.println("Echec de la connexion");
  }
}

void open_door(){
  
  Serial.println(">> Open");
  analogWrite(servoPin, 20);
  int i = 0;
  while(i < 12){
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    delay(100);
    i += 1;
  }
  analogWrite(servoPin,0);
}

void close_door(){
  Serial.println(">> Close");
  analogWrite(servoPin, 250);
  delay(2400);
  analogWrite(servoPin,0);
}

String res = "";
void loop() {
 
  // put your main code here, to run repeatedly:
  char c = 0;
  boolean extract = false;
  while(client.available()){
    c = client.read();
    if(c == '}'){
      extract = false;
    }
    if(extract){
       res += c;
    }
    if(c == '{'){
      extract = true;
    } 
  }  
  //Serial.println("Response :: ");
  //Serial.println(res);

  // reset packetBuffer array
  memset(packetBuffer, 0, sizeof(packetBuffer));
  
  int packetSize = Udp.parsePacket();
  if(packetSize){
    
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println(packetBuffer);

    switch(packetBuffer[0]){
      case '1':
        open_door();
      break;
      case '2':
        close_door();
      break;
    }
  }

}
