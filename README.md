# Braserinator

Este proyecto se ha realizado durante el master en la asignatura de Descripción de Sistemas Hadware en el que consistía en realizar un proyecto ubicuo.

https://s-computing.blogspot.com/2023/11/braserinator.html

## Escenario

El Sr. Heinz no soporta el frío que hace en su casa cuando llega de trabajar y lo único que desea es sentarse en el sofá y ver la televisión.

Sin embargo, Heinz sufre de dolores de espalda y tener que encender y apagar el brasero situado debajo de la mesa resulta incómodo, por lo que muchas veces antes de irse a dormir se descuida y lo deja encendido toda la noche.

Cuando Heinz se sienta a ver la televisión le gusta mantener la lampara del salón encendida pero tenue para poder descansar la vista por la noche, sin embargo, siempre está a máxima potencia porque su hija estudia allí por las tardes, así que muchas veces ni la enciende por pereza.

Hace poco, el Sr Heinz comenzó a utilizar uno de sus nuevos inventos para solucionar todas estas situaciones. Ahora, cuando llega a casa y se sienta en el sofá después de un duro día, el brasero se enciende automáticamente, sin necesidad de agacharse a encenderlo manualmente. De forma automática, las luces se encienden y bajan de intensidad porque se identifica que es Heinz el que se ha sentado en el sofá. Al mismo tiempo, se ha encendido el televisor para que Heinz elija un canal de televisión y vea su serie favorita sobre un ornitorrinco agente secreto.

## Resumen

El sistema a desarrollar consiste utilizar sensores en el salón o salita con un brasero. El sistema también se conectará con las lámparas de la sala y el televisor.

    Personalización de luces y temperatura: el sistema contará con la capacidad de conectarse a los teléfonos móviles de los usuarios para identificarlos y adaptar la iluminación y la temperatura del brasero de forma personalizada.
    Sistema de seguridad y ahorro energético: el sistema utilizará sensores de presión en los sillones junto a la mesa para detectar si hay o no una persona sentada y apagar el brasero y las luces si no hay nadie sentado a la mesa durante un tiempo.
    Encendido automático de televisión: el sistema podrá encender el televisor si detecta a un usuario sentado en el salón.


## Material utilizado

[Braserinator esquema general](../Braserinator/esquemas/braserinator_mqtt.drawio.png)

- 5 ESP32
- 1 sensor de temperatura
- 1 emisor de infrarrojos
- 1 sensor de luminosidad
- 1 anillo LED RGB
- 1 sensor de presión
- 2 resistencia de 10k Ohmios
- 1 RasberryPi 3 Model B


## Para poder recrear el sistema

- Se deberá cargar en cada una de las esp32 uno de programas que se encuentran en la carpeta */arduino*

- Se deberá pasar los ficheros que se encuentran en la carpeta */server* en la rasberry pi, además de tener instalado MariaDB y el broker de Mosquitto.

- Para la base de datos se encuentran en la carpeta */database* los script para MariaDB.

- En la carpeta */esquemas* se encuentran los esquemas del cableado de todos los esp32 y un esquema general que ilustra el cambio de información en el sistema.


## Aspectos a tener en cuenta 

- Para la identificación del usuario se hace a traves de bluetooh por lo que es necesario tener un dispositivo movil. El usuario antes tendrá que haber metido su configuración en el portal cautivo a traves del codigo qr que se encuentra en la carpeta */arduino/CaptivePortal* o accediendo a la ip 192.168.50.238 que está configurado el portal cautivo.

- La dirección del broker de mosquitto se tendrá que configurar por una nueva en cada uno de los esp32 que publique o se subcriba en algun topic, se tendrá entonces que cambiar la variable *mqtt_server* en los ficheros de los esp32.

- Para probar el servicio de encendido de la televisión, esta tendra que ser modelo LG, pues es el modelo que esta programada hasta la fecha.
