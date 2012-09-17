#include <Ethernet.h>
#include <SPI.h>
#include <aJSON.h>
#include <SimpleTimer.h>

/*
  Pin configuration
*/
int moistureInputPin = 0;
int moistureOutputPinTop = 2;
int moistureOutputPinBottom = 3;
int valveOutputPin = 9;

/*
  How often to check the plant needs watering
  and to send readings to the server
*/
unsigned long pollMillis = 60000; // 1 min


/*
  How dry the soil should get before watering.
  (Max) 1023 = Drier than the Atacama Desert
        400  = Damp
  (Min) 0    = Have you used copper instead of soil? 
*/
int waterAtMoisture = 1023 - 400;

/*
  Server connection details
*/
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte server[] = { 54, 247, 99, 12 };
byte ip[] = { 192, 168, 1, 102 }; 
byte submask[] = { 255, 255, 255, 0 };
byte gateway[] = { 192, 168, 1, 254 };
byte dns[] = { 192, 168, 1, 254 };

EthernetClient client;
SimpleTimer timer;

void setup() {
  pinMode(moistureOutputPinTop,OUTPUT);
  pinMode(moistureOutputPinBottom,OUTPUT);
  pinMode(valveOutputPin, OUTPUT);
  
  Serial.begin(9600);
  
  Ethernet.begin(mac, ip, dns, gateway, submask);
  
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  timer.setInterval(pollMillis, poll);
}

void loop() {  
  timer.run();
}

/*
  Main function that is run once per poll
*/
void poll() {
  Serial.print("Polling... ");
  
  int moisture = getSoilMoisture();
  int watered = 0;
  
  Serial.println(moisture);
  
  if(moisture > waterAtMoisture) {
    waterThePlant();
    watered = 1;
  }
  
  postToServer(moisture, watered);
}

/*
  Drives a current though two digital pins
  and reads the resistance through the
  analogue pin
*/
int getSoilMoisture(){
  //Drive a current through the divider in one direction
  digitalWrite(moistureOutputPinTop,HIGH);
  digitalWrite(moistureOutputPinBottom,LOW);
  delay(1000);

  //Take a reading
  int reading = analogRead(moistureInputPin);

  //Reverse the current
  digitalWrite(moistureOutputPinTop,LOW);
  digitalWrite(moistureOutputPinBottom,HIGH);
  delay(1000);

  //Stop the current
  digitalWrite(moistureOutputPinBottom,LOW);

  return reading;
}

/*
  Gives the plant a two second blast of water
*/
void waterThePlant() {
  digitalWrite(valveOutputPin, HIGH);
  delay(1000);
  digitalWrite(valveOutputPin, LOW);
}

/*
  Posts a JSON document to the cube server 
  (see http://square.github.com/cube/)
  
  The JSON document describes the soil
  moisture and whether the plant was
  watered.
  
  [{
    "data": {
      "moisture": 600,
      "watered": 0
    }
  }]
  
  Using int rather than boolean for 'watered'
  as cube metric querying is limited with booleans 
*/
void postToServer(int moisture, int watered) {
  while(!client.connected()) {
    client.stop();
    Serial.println("Connecting...");
    if (client.connect(server, 80)) {
      Serial.println("Connected");
    } else {
      Serial.println("Connection failed");
      delay(5000);
    }
  }
  
  aJsonObject* rootJson = aJson.createArray();
  
  aJsonObject* event = aJson.createObject();
  aJson.addItemToArray(rootJson, event);
  aJson.addStringToObject(event, "type", "moisture");
  aJsonObject* data = aJson.createObject();
  aJson.addItemToObject(event, "data", data);
  aJson.addNumberToObject(data, "moisture", moisture);
  aJson.addNumberToObject(data, "watered", watered);
  
  char* moistureJsonStr = aJson.print(rootJson);
  
  client.println("POST /1.0/event/put HTTP/1.0");
  client.println("Content-Length: " + String(strlen(moistureJsonStr)));
  client.println("Connection: keep-alive");
  client.println();
  client.println(moistureJsonStr);
}