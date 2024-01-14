#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <PubSubClient.h>

DNSServer dnsServer;
AsyncWebServer server(80);

String user_name;
int rojo;
int verde;
int azul;
int brillo;
int canal;
String marca_tv;

bool rojo_received=false;
bool verde_received=false;
bool azul_received=false;
bool brillo_received=false;
bool name_received = false;
bool canal_received=false;
bool marca_tv_received=false;

const char* mqtt_server = "192.168.50.141";
const char* topic = "esp32/preferencias";
const char* clientName = "espClient";

WiFiClient espClient;
PubSubClient client(espClient);

// Mensajes
long lastMsg = 0;
char msg[50];
int value = 0;


struct RGBColor {
  int r;
  int g;
  int b;
};

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>BRASERINATOR</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f5f5f5;
      margin: 20px;
    }

    h1 {
      color: #333;
    }

    form {
      background-color: #fff;
      padding: 20px;
      border-radius: 10px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      max-width: 400px;
      margin: 0 auto;
    }

    label {
      display: block;
      margin-top: 10px;
      font-weight: bold;
    }

    input,
    select {
      width: 100%;
      padding: 8px;
      margin-top: 5px;
      margin-bottom: 15px;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }

    input[type="range"] {
      width: calc(100% - 16px);
      margin-left: 8px;
    }

    input[type="range"]::after {
      content: attr(value) "%";
      position: absolute;
      top: -20px;
      right: 0;
      font-size: 12px;
      color: #555555;
    }

    input[type="submit"] {
      background-color: #4caf50;
      color: white;
      padding: 10px 15px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
    }

    input[type="submit"]:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <h1>Formulario de registro en Braserinator</h1>
  <form action="/get">
    <label for="name">Nombre del dispositivo:</label>
    <input type="text" name="name" required>

    <h2>Configuración de las luces</h2>

    <label for="colorPicker">Seleccionar color:</label>
    <input type="color" id="colorPicker" name="colorPicker" onchange="updateColor(this.value)">

    <label for="brillo">Brillo (<span id="brilloValue">50</span>%):</label>
    <input type="range" name="brillo" min="0" max="100" value="50" oninput="updateBrilloValue(this.value)" required>

    <h2>Configuración del canal de televisión</h2>

    <label for="canal">Seleccionar canal:</label>
    <input type="number" id="canal" name="canal" max="9999" required>

    <label for="marca_tv">Marca de la televisión:</label>
    <select id="marca_tv" name="marca_tv">
            <option value="LG">LG</option>
            <option value="Samsung">Samsung</option>
            <option value="TD Systems">TD Systems</option>
        </select>
        <br>
    <input type="submit" value="Guardar configuración">
  </form>

  <script>
    function updateBrilloValue(value) {
      const brilloValue = document.getElementById('brilloValue');
      brilloValue.textContent = value;
    }

    function updateColor(hexColor) {
      // Convierte el color hexadecimal a RGB
      const rgbColor = hexToRgb(hexColor);
            
      // Muestra el valor RGB en el elemento con id "rgbValue"
      console.log(`RGB(${rgbColor.r}, ${rgbColor.g}, ${rgbColor.b})`);
    }

    function hexToRgb(hex) {
      // Elimina el carácter '#' si está presente
      hex = hex.replace(/^#/, '');

      // Convierte el valor hexadecimal a valores RGB
      const bigint = parseInt(hex, 16);
      const r = (bigint >> 16) & 255;
      const g = (bigint >> 8) & 255;
      const b = bigint & 255;

      return { r, g, b };
    }
  </script>
</body>
</html>)rawliteral";


RGBColor hexToRgb(String hexColor) {
  hexColor.replace("#", "");

  long number = strtol(hexColor.c_str(), NULL, 16);

  RGBColor rgbColor;
  rgbColor.r = (number >> 16) & 0xFF;
  rgbColor.g = (number >> 8) & 0xFF;
  rgbColor.b = number & 0xFF;

  return rgbColor;
}


class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html); 
  }
};

void setupServer(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html); 
      Serial.println("Client Connected");
  });
    
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {//Aqui se devuelven los valores de los datos introducidos por el cliente (formulario)
      String inputMessage;
      String inputParam;
  
      if (request->hasParam("name")) {
        inputMessage = request->getParam("name")->value();
        inputParam = "name";
        user_name = inputMessage;
        Serial.println(inputMessage);
        name_received = true;
      }
      if (request->hasParam("colorPicker")) {
        inputMessage = request->getParam("colorPicker")->value();
        inputParam = "colorPicker";

        // Convierte el valor hexadecimal a RGB
        const RGBColor rgbColor = hexToRgb(inputMessage);

        // Asigna los valores RGB a las variables correspondientes
        rojo = rgbColor.r;
        verde = rgbColor.g;
        azul = rgbColor.b;

        // Muestra los valores RGB en la consola serial
        Serial.println("Rojo: " + String(rojo));
        Serial.println("Verde: " + String(verde));
        Serial.println("Azul: " + String(azul));

        // Indica que se han recibido los valores
        rojo_received = true;
        verde_received = true;
        azul_received = true;
      }

      if (request->hasParam("brillo")) {
        inputMessage = request->getParam("brillo")->value();
        inputParam = "brillo";
        brillo = inputMessage.toInt();
        Serial.println(inputMessage);
        brillo_received = true;
      }
      if (request->hasParam("canal")) {
        inputMessage = request->getParam("canal")->value();
        inputParam = "canal";
        canal = inputMessage.toInt();
        Serial.println(canal);
        canal_received = true;
      }
      
      if (request->hasParam("marca_tv")) {
        inputMessage = request->getParam("marca_tv")->value();
        inputParam = "marca_tv";
        marca_tv = inputMessage;
        Serial.println(inputMessage);
        marca_tv_received = true;
      }

      request->send(200, "text/html", "The values entered by you have been successfully sent to the device <br><a href=\"/\">Return to Home Page</a>");
  });
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageSofa;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageSofa += (char)message[i];
  }
  Serial.println();

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
  Serial.print("Mensaje ");
  Serial.print(value.c_str());
  Serial.print(" enviado a ");
  Serial.println(topic.c_str());

}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Setting up Station Mode");

  IPAddress staticIP(192, 168, 50, 238); // Cambia la IP según tus necesidades
  IPAddress gateway(192, 168, 50, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.config(staticIP, gateway, subnet);

  // Connect to an existing Wi-Fi network
  const char* ssid = "realme";
  const char* password = "1234567890";

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Setting up Async WebServer");
  setupServer();
  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.localIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_STA_FILTER); // only when requested from station
  server.begin();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(1500);

  Serial.println("All Done!");
}


void loop(){
  dnsServer.processNextRequest();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(name_received && azul_received && rojo_received && verde_received && brillo_received && canal_received && marca_tv_received){
      brillo=round(brillo*2.55);

      Serial.print("El nombre del dispositivo es ");Serial.println(user_name);
      Serial.print("El color azul recibido para baja luminosidad es ");Serial.println(azul);
      Serial.print("El color rojo recibido para baja luminosidad es ");Serial.println(rojo);
      Serial.print("El color verde recibido para baja luminosidad es ");Serial.println(verde);
      Serial.print("El brillo recibido es ");Serial.println(brillo);
      Serial.print("El canal recibido es ");Serial.println(canal);
      Serial.print("La marca de la televisión recibida es ");Serial.println(marca_tv);

      name_received = false;
      azul_received=false;
      rojo_received=false;
      verde_received=false;
      brillo_received=false;
      canal_received=false;
      marca_tv_received=false;

      String preferencias=user_name+";"+rojo+";"+verde+";"+azul+";"+brillo+";"+canal+";"+marca_tv;

      mqttEmit(topic, preferencias);

      Serial.println("We'll wait for the next client now");
    }
}