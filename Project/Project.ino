/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/
#include <Smoothed.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
////
const int trigPin = 2;
const int echoPin = 5;
Smoothed <float> mySensor; 
Smoothed <float> mySensor2;  
////
// Replace the next variables with your SSID/Password combination
const char* ssid = "RouteryPi";
const char* password = "raspberry";
const char* mqtt_server = "192.168.1.2";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//uncomment the following lines if you're using SPI
/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

// LED Pin
const int ledPin = 2;
////
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3        /* Time ESP32 will go to sleep (in seconds) */
//RTC_DATA_ATTR int bootCount = 0;
///////
void setup() {
  Serial.begin(9600);
mySensor.begin(SMOOTHED_AVERAGE, 10);    // Initialise the first sensor value store. We want this to be the simple average of the last 10 values.
mySensor2.begin(SMOOTHED_EXPONENTIAL, 10);  // Initialise the second sensor value store. We want this one to be a simple linear recursive exponential filter. 
        // We set the filter level to 10. Higher numbers will result in less filtering/smoothing. Lower number result in more filtering/smoothing
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
   long duration; 
float distance1; 
 ///////////// Clears the trigPin
digitalWrite(trigPin, LOW);
delayMicroseconds(20);
// Sets the trigPin on HIGH state for 10 micro seconds
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);
// Reads the echoPin, returns the sound wave travel time in microseconds
duration = pulseIn(echoPin, HIGH);
// Calculating the distance
distance1=((duration/58.2)/100);
//distance1+=2;
// Prints the distance on the Serial Monitor
Serial.print("Distance: ");
Serial.print(distance1);
Serial.println(" m");

//filter
 float currentSensorValue = distance1;

   // Add the new value to both sensor value stores
  mySensor.add(currentSensorValue);
  mySensor2.add(currentSensorValue); 

  // Get the smoothed values
  float smoothedSensorValueAvg = mySensor.get();
  float smoothedSensorValueExp = mySensor2.get();    
  ///////////////////
  delay(500);
    
    // Convert the value to a char array
    char curString[8];
    dtostrf(currentSensorValue, 1, 2, curString);
    Serial.print("Distance: ");
    Serial.println(curString);
    client.publish("esp32/ultrasonic", curString);
  
  char avgString[8];
    dtostrf(smoothedSensorValueAvg, 1, 2, avgString);
    Serial.print("Distance: ");
    Serial.println(avgString);
    client.publish("esp32/ultrasonic2", avgString);
  
  char expString[8];
    dtostrf(smoothedSensorValueExp, 1, 2, expString);
    Serial.print("Distance: ");
    Serial.println(expString);
    client.publish("esp32/ultrasonic3", expString);
  
  delay(500);
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //esp_deep_sleep_start();
}
}
