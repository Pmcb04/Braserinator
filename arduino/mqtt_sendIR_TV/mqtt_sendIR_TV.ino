#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <ArduinoJson.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>
//#include <Wire.h>

//#define fsrpin A0 //Sensor de presión

// variable de estado para saber si la televisión esta encendida
bool isTvOn;

// // Wifi config
const char* ssid = "realme";
const char* password = "1234567890";

// Wifi config
// const char* ssid = "MOVISTAR_F8D0";
// const char* password = "Pbm74RC9f5mV3YsYWiuq";

// MQTT config
const char* mqtt_server = "192.168.50.141";
const char* subtopic = "esp32/tv";
const char* clientName = "espClient";

WiFiClient espClient;
PubSubClient client(espClient);


// An IR detector/demodulator is connected to GPIO pin 14(D5 on a NodeMCU
// board).
// Note: GPIO 16 won't work on the ESP8266 as it does not have interrupts.
// Note: GPIO 14 won't work on the ESP32-C3 as it causes the board to reboot.
#ifdef ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = 10;  // 14 on a ESP32-C3 causes a boot loop.
#else  // ARDUINO_ESP32C3_DEV
const uint16_t kRecvPin = D2;
#endif  // ARDUINO_ESP32C3_DEV

const uint16_t kIrLed = D6;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.
IRrecv irrecv(kRecvPin);

// Tamaño del buffer para recibir mensajes MQTT
const int buffer_size = 256;
char buffer[buffer_size];

decode_results results;

// Mensajes
long lastMsg = 0;
char msg[50];
int value = 0;

class LGTVRemote {

  private:
    enum LG_IR_CODES {
      ON_OFF = 0x20DF10EF,
      TV_RADIO = 0x20DF0FF0,
      VOLUME_UP = 0x20DF40BF,
      VOLUME_DOWN = 0x20DFC03F,
      MUTE = 0x20DF906F,
      NUMBER_0 = 0x20DF08F7,
      NUMBER_1 = 0x20DF8877,
      NUMBER_2 = 0x20DF48B7,
      NUMBER_3 = 0x20DFC837,
      NUMBER_4 = 0x20DF28D7,
      NUMBER_5 = 0x20DFA857,
      NUMBER_6 = 0x20DF6897,
      NUMBER_7 = 0x20DFE817,
      NUMBER_8 = 0x20DF18E7,
      NUMBER_9 = 0x20DF9867,
    };

  public:
    LGTVRemote() {
      irsend.begin();
    }

    // Método para enviar un código IR
    void sendIRCode(unsigned long code) {
      Serial.print("Send code ");
      Serial.println(code);
      irsend.sendNEC(code, 32);
      delay(50); // Espera 50ms entre envíos para evitar repeticiones no deseadas
    }

    // Métodos para cada tecla con su código correspondiente
    void powerOnOff() {
      sendIRCode(LG_IR_CODES::ON_OFF);
    }

    void tvRadio() {
      sendIRCode(LG_IR_CODES::TV_RADIO);
    }

    void volumeUp(int points) {
      if (points < 0) points = 0;
      for (int i = 0; i < points; i++) {
        sendIRCode(LG_IR_CODES::VOLUME_UP);
        delay(50);
      }
    }

    void volumeDown(int points) {
      if (points < 0) points = 0; 
      for (int i = 0; i < points; i++) {
        sendIRCode(LG_IR_CODES::VOLUME_DOWN);
        delay(50);
      }
    }

    void mute() {
      Serial.println("Send MUTE code");
      sendIRCode(LG_IR_CODES::MUTE);
    }

    void number0() {
      Serial.println("Send NUMBER_0 code");
      sendIRCode(LG_IR_CODES::NUMBER_0);
    }

    void number1() {
      Serial.println("Send NUMBER_1 code");
      sendIRCode(LG_IR_CODES::NUMBER_1);
    }

    void number2() {
      Serial.println("Send NUMBER_2 code");
      sendIRCode(LG_IR_CODES::NUMBER_2);
    }

    void number3() {
      Serial.println("Send NUMBER_3 code");
      sendIRCode(LG_IR_CODES::NUMBER_3);
    }

    void number4() {
      Serial.println("Send NUMBER_4 code");
      sendIRCode(LG_IR_CODES::NUMBER_4);
    }

    void number5() {
      Serial.println("Send NUMBER_5 code");
      sendIRCode(LG_IR_CODES::NUMBER_5);
    }

    void number6() {
      Serial.println("Send NUMBER_6 code");
      sendIRCode(LG_IR_CODES::NUMBER_6);
    }

    void number7() {
      Serial.println("Send NUMBER_7 code");
      sendIRCode(LG_IR_CODES::NUMBER_7);
    }

    void number8() {
      Serial.println("Send NUMBER_8 code");
      sendIRCode(LG_IR_CODES::NUMBER_8);
    }

    void number9() {
      Serial.println("Send NUMBER_9 code");
      sendIRCode(LG_IR_CODES::NUMBER_9);
    }

        // Función para obtener la llave a partir del valor del enum
    const char* getKeyFromValue(unsigned long value) {
      switch (value) {
        case ON_OFF: return "ON_OFF";
        case TV_RADIO: return "TV_RADIO";
        case VOLUME_UP: return "VOLUME_UP";
        case VOLUME_DOWN: return "VOLUME_DOWN";
        case MUTE: return "MUTE";
        case NUMBER_0: return "NUMBER_0";
        case NUMBER_1: return "NUMBER_1";
        case NUMBER_2: return "NUMBER_2";
        case NUMBER_3: return "NUMBER_3";
        case NUMBER_4: return "NUMBER_4";
        case NUMBER_5: return "NUMBER_5";
        case NUMBER_6: return "NUMBER_6";
        case NUMBER_7: return "NUMBER_7";
        case NUMBER_8: return "NUMBER_8";
        case NUMBER_9: return "NUMBER_9";
        default: return "UNKNOWN"; // Valor no encontrado en el enum
      }
    }

    // Función sobrecargada que utiliza el enum directamente
    const char* getKeyFromValue(LG_IR_CODES value) {
      return getKeyFromValue(static_cast<unsigned long>(value));
    }

    void sendChannel(String channel){
      // Obtener la longitud del String
      int longitud = channel.length();
      Serial.print("longitud del canal recibido ");
      Serial.println(longitud);

      //Procesar el String carácter por carácter
      for (int i = 0; i < longitud; i++) {
        char character = channel.charAt(i); // Obtener el carácter en la posición 'i'
        switch (character) {
          case '0': number0(); break;
          case '1': number1(); break;
          case '2': number2(); break;
          case '3': number3(); break;
          case '4': number4(); break;
          case '5': number5(); break;
          case '6': number6(); break;
          case '7': number7(); break;
          case '8': number8(); break;
          case '9': number9(); break;
          default: Serial.println("UNKNOWN"); break; // Valor no encontrado en el enum
        }
        delay(1500);
      }
    }
};


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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");

  String eventOffTV;
  LGTVRemote myTVlg;

  for (int i = 0; i < length; i++) {
    eventOffTV += (char)payload[i];
  }
  Serial.println(eventOffTV);

  if(eventOffTV.compareTo("OFF")==0){
    if(isTvOn){
      myTVlg.powerOnOff();
      Serial.println("Apgando tv..");
      isTvOn = false;
    }
  }
  else{
    // Copiar el payload a un buffer de caracteres
  memcpy(buffer, payload, length);
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
  String marca = doc["marca"];
  String canal_favorito = doc["canal_favorito"];
  String event = doc["event"];

  // envio del canal a TV LG
  if (marca == "LG"){
    if(event == "on" && !isTvOn){
      myTVlg.powerOnOff();
      Serial.println("Mandando tv LG");
      Serial.println("Encendiendo tv...");
      isTvOn = true;
      Serial.println("TV on");
      delay(15000);
    }else if(event == "channel"){
      myTVlg.sendChannel(canal_favorito);
    }
  }

  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESPClient";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(subtopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqttEmit(String topic, String value){
  client.publish((char*) topic.c_str(), (char*) value.c_str());
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  isTvOn = false;
  setupWifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
 
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}