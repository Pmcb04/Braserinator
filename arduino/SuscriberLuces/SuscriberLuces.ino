#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h>
#endif


const int ldr=A0;

#define PIN 4
#define NUMPIXELS 8
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Variable de lectura del sensor de luminosidad
int valor;
// Variable de estado de la luz. FALSE=apagado, TRUE=encendido
bool luz;
// Contador de lecturas en las que no hay nadie
int nadie; 

// Wifi config
const char* ssid = "realme";
const char* password = "1234567890";

// MQTT config
const char* mqtt_server = "192.168.50.141";
const char* topic = "esp32/luces";
const char* clientName = "espClientLuces";

// Tamaño del buffer para recibir mensajes MQTT
const int buffer_size = 256;
char buffer[buffer_size];


WiFiClient espClient;
PubSubClient client(espClient);

// Mensajes
long lastMsg = 0;
char msg[50];
int value = 0;

// Variables para almacenar los strings divididos
int brillo2;

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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  //Mensaje recibido por MQTT
  String messageLuz;
  
  for (int i = 0; i < length; i++) {
    messageLuz += (char)message[i];
  }
  Serial.println(messageLuz);

  if(messageLuz.compareTo("OFF")==0){
    luz=false;
  }
  else{
    luz=true;
  }
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/luces" && luz==true) {
    /*Serial.print("Changing output to ");
    char delimitador = ';';

    int indiceDelimitador = messageLuz.indexOf(delimitador);

    rojo = messageLuz.substring(0, indiceDelimitador).toInt();
    messageLuz = messageLuz.substring(indiceDelimitador + 1);

    indiceDelimitador = messageLuz.indexOf(delimitador);
    verde = messageLuz.substring(0, indiceDelimitador).toInt();
    messageLuz = messageLuz.substring(indiceDelimitador + 1);

    indiceDelimitador = messageLuz.indexOf(delimitador);
    azul = messageLuz.substring(0, indiceDelimitador).toInt();
    messageLuz = messageLuz.substring(indiceDelimitador + 1);

    indiceDelimitador = messageLuz.indexOf(delimitador);
    brillo = messageLuz.substring(0, indiceDelimitador).toInt();*/

    memcpy(buffer, message, length);
    buffer[length] = '\0'; // Asegurarse de agregar un carácter nulo al final

    // Deserializar el JSON
    StaticJsonDocument<buffer_size> doc;
    DeserializationError error = deserializeJson(doc, buffer);

    // Verificar errores de deserialización
    if (error) {
      Serial.print("Error al deserializar JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Acceder a los valores del JSON (en este ejemplo, se asume que es una tupla)
    int rojo = doc["red"];
    int verde = doc["green"];
    int azul = doc["blue"];
    int brillo = doc["brillo"];

    brillo2=brillo;

    pixels.setPixelColor(0,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(1,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(2,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(3,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(4,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(5,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(6,pixels.Color(rojo,verde,azul));
    pixels.setPixelColor(7,pixels.Color(rojo,verde,azul));
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientName)) {
      Serial.println("connected");
      client.subscribe(topic);
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

void setup() {

  pixels.begin();

  luz=false;  
  pinMode(ldr, INPUT);
  Serial.begin(9600);
  nadie = 0;

  // Servidor MQTT
  setupWifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(1500);
}

void loop() {
  // Wait a few seconds between measurements.
  delay(1000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(luz){
    valor = analogRead(ldr);
    if (valor > 1500) {
      pixels.setBrightness(brillo2/5);
      pixels.show();
    }
    if (valor <= 1500 && valor>=500) {
      pixels.setBrightness(brillo2/2);
      pixels.show();
    }
    if (valor<500) {
      pixels.setBrightness(brillo2);
      pixels.show();
    }
    delay(500);
  }
  else{
    pixels.clear();
    pixels.show();
  }
  
}