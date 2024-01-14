#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

HardwareSerial SerialPort(2); // use UART2

#define DHTTYPE DHT11   // DHT 11
#define DHTPin 23 //Sensor de temperatura

// Señal brasero
char number  = ' ';
bool brasero;

// Wifi config
//const char* ssid = "iFer";
//const char* password = "pokeFer08";
const char* ssid = "realme";
const char* password = "1234567890";

// MQTT config
//const char* mqtt_server = "test.mosquitto.org";
const char* mqtt_server = "192.168.50.141";
const char* userTopic = "esp32/user";
const char* lucesTopic = "esp32/luces";
const char* tvTopic = "esp32/tv";
const char* clientName = "espClient";

WiFiClient espClient;
PubSubClient client(espClient);

// Dispositivo de temperatura y humedad
DHT dht(DHTPin, DHTTYPE);

void setupWifi() {
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

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientName)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void mqttEmit(String topic, String value){
  client.publish((char*) topic.c_str(), (char*) value.c_str());
}

//RECEPTOR
void setup() {
  Serial.begin(9600);
  SerialPort.begin(9600, SERIAL_8N1, 16, 17);

  // Led integrado que simula el estado del brasero
  pinMode(BUILTIN_LED, OUTPUT);
  brasero = false; 

  // Servidor MQTT
  setupWifi();
  client.setServer(mqtt_server, 1883);

  //DHT
  dht.begin();

  Serial.println("Contempla... ¡El BRASERINATOR!");
  delay(1500);
}

void loop() {
  delay(500);

  Serial.print("*");
  if (SerialPort.available()) {
    char number = SerialPort.read();
    Serial.print("Mensaje recibido: ");
    Serial.println(number);
    if (number == '1') {
      Serial.println("Recibida señal encendido");
      brasero = true;
      //mqttEmit("esp32/brasero", "ON");
      digitalWrite(LED_BUILTIN, HIGH);

      while (!SerialPort.available()){ //esperar a un segundo mensaje
        Serial.println("Esperando recibir dispositivo BT...");
        delay(500);
      }
      // Lee el nombre del dispositivo Bluetooth
      String deviceName = SerialPort.readString();
      Serial.print("Dispositivo Bluetooth recibido: ");
      Serial.println(deviceName);

      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      if(deviceName.equals("nada")){
        Serial.print("No se encontraron dispositivos Bluetooth");
        mqttEmit(userTopic, "nada");
      }
      else{
        //Enviar por mqtt
        Serial.println("Enviando dispositivo para cargar configuración personalizada");
        mqttEmit(userTopic, deviceName);
        //prueba
        //mqttEmit("esp32/luces", "{\"red\": 45, \"green\": 190, \"blue\": 79, \"brillo\": 100}");
      }
      
    }
    if (number == '0') {
      Serial.println("Recibida señal apagado");
      delay(500);

      if (!client.connected()) {
        reconnect();
      }
      client.loop();
      
      if(brasero){
        brasero = false;
        //mqttEmit("esp32/brasero", "OFF"); //Notificar al server de brasero apagado
      }
      mqttEmit(lucesTopic, "OFF"); // Mandar señal apagado de luces
      mqttEmit(tvTopic, "OFF"); // Mandar señal apagado de luces

      Serial.println("Brasero y luces apagadas.");

      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  //Solamente si el brasero está encendido se comprueba la temperatura por seguridad
  if(brasero){
    delay(500);
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    Serial.print(" Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C ");

    if(t > 25){ //Valor de temperatura límite a 25 para facilitar pruebas
      Serial.println("Temperatura máxima alcanzada, apagando brasero... ");
      digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
      brasero = false;
    }
  }
}