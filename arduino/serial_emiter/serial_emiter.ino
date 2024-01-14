#include <HardwareSerial.h>
#include <BluetoothSerial.h>

HardwareSerial SerialPort(2); // use UART2

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

#define BT_DISCOVER_TIME	10000

//CONFIGURACION SENSOR SOFA
#define fsrpin A0 //Sensor de presión
// Variable de lectura del sensor
int readsofa;
// Variable de estado de brasero. FALSE=apagado, TRUE=encendido
bool brasero;
// Contador de lecturas en las que no hay nadie sentado
int nadie; 

// EMISOR
void setup()  {
  Serial.begin(9600);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  SerialPort.begin(9600, SERIAL_8N1, 16, 17); 

  //pinMode(LED_BUILTIN, OUTPUT); // LED INTEGRADO QUE SUSTITUYE AL BRASERO
  brasero = false; // podría tomar el estado del brasero del último estado almacenado en servidor central
  nadie = 0;
} 
void loop()  { 
  delay(1000);

  // Lectura del sensor de fuerza
  readsofa = analogRead(fsrpin);
  // Imprimir lectura de sensor de fuerza 
  Serial.print("Sensor de fuerza = ");
  // Print the fsrreading:
  Serial.print(readsofa);
  //Comportamiento según valor leído.
  if (readsofa >= 2000) {
      // Se detecta que se ha sentado alguien en el sofá
      Serial.println(" - Persona sentada ");
      nadie = 0;
      if(!brasero){
        Serial.print(" - Encendiendo brasero... ");
        //Enciende brasero (en el otro ESP32)
        //digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
        brasero = true;
        //Informamos al otro ESP32 de encender brasero
        Serial.println("Emitiendo señal encendido");
        SerialPort.print(1);
        delay(500);

        //Buscar dispositivos BT...
        Serial.println("Starting discover...");
        BTScanResults *pResults = SerialBT.discover(BT_DISCOVER_TIME);
        if (pResults->getCount() > 0){
          pResults->dump(&Serial);
          BTAdvertisedDevice *pDevice;
          //BTAdvertisedDevice *dispActual;
          //bool encontrado=false;
          // Usamos el dispositivo detectado primero
          pDevice = pResults->getDevice(0);
          Serial.print("Detectado dispositivo con nombre: ");
          Serial.println(pDevice->getName().c_str());
          //Enviamos el nombre por el puerto serie al otro dispositivo
          SerialPort.print(pDevice->getName().c_str());
          //MySerial.write(pDevice->getName().c_str());
        }
        else{
          Serial.println("No se encontraron dispositivos BT.");
          SerialPort.print("nada");
        }
      }
  }
  if (brasero & readsofa < 2000) {
    Serial.println(" - Nadie ");
    // Si hay varias lecturas nulas seguidas durante X tiempo también se apagará el brasero.
    nadie+=1;
    Serial.print("Lecturas nulas: ");
    Serial.println(nadie);
  }
  if (brasero & nadie >= 10) {
    Serial.print("Tiempo de espera máximo alcanzado, apagando brasero... ");
    //Apagamos el brasero
    //digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    brasero = false;
    //Informamos al otro ESP32 de apagar brasero
    Serial.println("Emitiendo señal apagado");
    SerialPort.print(0);
    delay(500);
  }
  Serial.println();
}