## Plantilla T1

### Integrantes

- Francisco Galdames
- Valentina Vásquez
- Vicente Soto

---


# Código Raspberry
En la raspberry se inician 2 threads en el los cuales se atienden las conexiones UDP y TCP. 

- En el primero de ellos se aceptan las conexiones del ESP32 y se le responde con el protocolo, el transport layer y el id del mensaje a utilizar. Si la conexión es UDP entonces se cierra la conexión TCP ya que la comunicación debera seguir en el otro thead. De lo contrario recibe el mensaje (si es protocolo 4 entonces se espera a recibir la totalidad pues puede venir fragmentado, no funciona del todo bien).
- En el segundo se aceptan las conexiones UDP y una vez recibido un mensaje se verifica si la ultima tecla presiona es la t, indicando cambio de transport layer al enviar un mensaje a la ESP32.

En cualquiera de los 2 casos se parsea el mensaje mediante las funciones que reciben la data cruda y la transforman a un diccionario almacenando headers y datos. Finalmente se verifica si la longitud del mensaje es igual a la longitud indicada en los headers, en cuyo caso se crea la entrada en la tabla de datos, en caso contrario se descarta el paquete y se crea una entrada en la tabla loss. Para obtener el timestamp del mensaje se tiene como referencia el mensaje anterior del esp32.

# Código ESP32

En la esp32 se realiza la conexión por TCP al servidor, una vez aceptada se espera a recibir el protocolo, el transport layer y un id para el mensaje a enviar. 

- Si es TCP se utiliza el mismo socket y se crean y envian los datos (en el protocolo 4 se envia en paquetes de 1000). Finalmente se utiliza un deep_sleep.

- Si es UDP se crea un nuevo socket y se crean y envian los datos cada 3 segundos, intentando recibir un mensaje de vuelta para indicar el cambio de protocolo a TCP, si lo recibe se hace un deep_sleep si no se continua en el ciclo



## Comandos de docker


### Iniciar la base de datos

```bash
docker compose up -d
```

### Detener la base de datos

```bash
docker compose down
```

### Borrar la base de datos

```bash
docker compose down 
docker volume rm t1_postgres_data_iot
```