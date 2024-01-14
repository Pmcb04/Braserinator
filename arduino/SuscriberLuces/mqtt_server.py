import time
import json
from paho.mqtt import client as mqtt_client
from database import DatabaseManager


BROKER = 'localhost'
PORT = 1883

TOPIC_LIGHT = "esp32/luces"                    # Topic al que publica el servidor sobre persona detectada, suscribe ESP32 con luces y tv
TOPIC_TV = "esp32/tv"                        # Topic al que publica el servidor sobre persona detectada, suscribe ESP32 con luces y tv
TOPIC_CAPTIVE_PORTAL = "esp32/preferencias"   # Topic al que publica el servidor sobre las preferencias del portal cautivo, suscribe el servidor
TOPIC_USER = "esp32/user"                           # Topic al que se va a subcribir el servidor esperando el nombre de usuario

isUserSit = False

# Ejemplo de uso
databaseManager = DatabaseManager()

# Generate a Client ID with the client prefix.
client_id = 'client-{random.randint(0, 1000)}'


def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    # client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(BROKER, PORT)
    return client

def publish(client, topic, msg):
#def publish(client):
    time.sleep(1)

    result = client.publish(topic, msg)
    # result: [0, 1]
    status = result[0]
    if status == 0:
        print(f"Send `{msg}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        print(f"Received `{msg.payload.decode()}` from `{msg.topic}` topic")
        if(msg.topic==TOPIC_USER):
            nombre_usuario = msg.payload.decode()
            print("LLega ", nombre_usuario)
            id_usuario = databaseManager.obtener_id_usuario_por_nombre(nombre_usuario)
            print("Tiene id_usuario ", id_usuario)
            if (id_usuario != None):
                datos_luces = databaseManager.obtener_datos_luces(id_usuario)
                datos_luces_json = {"red": datos_luces[0], "green": datos_luces[1], "blue": datos_luces[0], "brillo": datos_luces[1]}
                publish(client, TOPIC_LIGHT, json.dumps(datos_luces_json))
                datos_tv = databaseManager.obtener_datos_television(id_usuario)
                datos_tv_json = {"marca": datos_tv[0], "canal_favorito": datos_tv[1], "event": "on"}
                publish(client, TOPIC_TV, json.dumps(datos_tv_json))
                datos_tv_json = {"marca": datos_tv[0], "canal_favorito": datos_tv[1], "event": "channel"}
                publish(client, TOPIC_TV, json.dumps(datos_tv_json))
            else:
                datos_luces_json = {"red": "255", "green": "255", "blue": "255", "brillo": "30"}
                publish(client, TOPIC_LIGHT, json.dumps(datos_luces_json))
                datos_tv_json = {"marca": "LG", "canal_favorito": None, "event": "on"}
                publish(client, TOPIC_TV, json.dumps(datos_tv_json))

        elif(msg.topic==TOPIC_CAPTIVE_PORTAL):
            preferencias=msg.payload.decode()
            databaseManager.insertar_datos(preferencias)
            


    client.subscribe(TOPIC_USER)
    client.subscribe(TOPIC_CAPTIVE_PORTAL)
    client.on_message = on_message

def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()
    # Cerrar la conexión
    # databaseManager.cerrar_conexion()


if __name__ == '__main__':
    run()
