import mysql.connector

class DatabaseManager:

    def __init__(self):
        # Inicializar la conexión a la base de datos en el constructor
        self.conexion = mysql.connector.connect(
            host="localhost",
            user="root",
            password="",
            database="braserinator"
        )
        # Crear un objeto cursor para ejecutar consultas
        self.cursor = self.conexion.cursor()

    def obtener_id_usuario_por_nombre(self, nombre_usuario):
        # Consulta para obtener el ID del usuario por su nombre
        consulta_id_usuario = "SELECT id FROM usuarios WHERE nombre = %s"
        self.cursor.execute(consulta_id_usuario, (nombre_usuario,))
        
        # Obtener el resultado de la consulta
        resultado = self.cursor.fetchone()

        # Si se encontró un resultado, obtener el ID del usuario
        id_usuario = resultado[0] if resultado else None

        return id_usuario

    def insertar_datos(self, preferencias):
        # Separar el string en una lista usando la coma como delimitador
        valores_lista = preferencias.split(';')

        # Crear la consulta de inserción para el usuario
        consulta_insercion_usuario = "INSERT INTO usuarios (nombre) VALUES (%s)"
        nombre_usuario = valores_lista[0]
        insertar_usuario = (nombre_usuario,)
        self.cursor.execute(consulta_insercion_usuario, insertar_usuario)

        # Hacer commit para guardar los cambios en la base de datos
        self.conexion.commit()

        # Consulta para obtener el ID del usuario recién insertado
        consulta_id_usuario = "SELECT id FROM usuarios WHERE nombre = %s"
        self.cursor.execute(consulta_id_usuario, insertar_usuario)

        # Obtener el resultado de la consulta
        resultado = self.cursor.fetchone()

        # Si se encontró un resultado, obtener el ID del usuario
        id_usuario = resultado[0] if resultado else None

        # Crear la consulta de inserción para las luces
        consulta_insercion_luces = "INSERT INTO comportamiento_luces (red, green, blue, brillo, id_usuario) VALUES (%s, %s, %s, %s, %s)"
        insertar_luces = (valores_lista[1], valores_lista[2], valores_lista[3], valores_lista[4], id_usuario,)
        self.cursor.execute(consulta_insercion_luces, insertar_luces)

        # Crear la consulta de inserción para la televisión
        consulta_insercion_tv = "INSERT INTO television (marca, canal_favorito, id_usuario) VALUES (%s, %s, %s)"
        insertar_tv = (valores_lista[6], valores_lista[5], id_usuario,)
        self.cursor.execute(consulta_insercion_tv, insertar_tv)

        # Hacer commit para guardar los cambios en la base de datos
        self.conexion.commit()

    def obtener_datos_television(self, id_usuario):
        # Consulta para obtener los datos de la televisión del usuario
        consulta_television = "SELECT marca, canal_favorito FROM television WHERE id_usuario = %s"
        self.cursor.execute(consulta_television, (id_usuario,))
        # Obtener los resultados de la consulta
        resultados_television = self.cursor.fetchone()
        return resultados_television if resultados_television else None

    def obtener_datos_luces(self, id_usuario):
        # Consulta para obtener los datos de las luces del usuario
        consulta_luces = "SELECT red, green, blue, brillo FROM comportamiento_luces WHERE id_usuario = %s"
        self.cursor.execute(consulta_luces, (id_usuario,))
        # Obtener los resultados de la consulta
        resultados_luces = self.cursor.fetchone()
        return resultados_luces if resultados_luces else None

    def cerrar_conexion(self):
        # Cerrar el cursor y la conexión al finalizar las operaciones
        self.cursor.close()
        self.conexion.close()