from peewee import Model, PostgresqlDatabase, DateTimeField, CharField, IntegerField

# Configuración de la base de datos
db_config = {
    'host': 'localhost', 
    'port': 5432, 
    'user': 'postgres', 
    'password': 'postgres', 
    'database': 'db'
}
db = PostgresqlDatabase(**db_config)

# Definición de un modelo
class BaseModel(Model):
    class Meta:
        database = db

# Ahora puedes definir tus modelos específicos heredando de BaseModel
# y db estará conectado al servicio de PostgreSQL cuando realices operaciones de base de datos.

# Definicion modelo Datos
# Datos recibidos, timestamp, Id_device, MAC
class Datos(BaseModel):
    timestamp = DateTimeField()
    ID_device = CharField()
    MAC = CharField()

# Definicion modelo Logs
# Conexiones recibidas por el servidor: Id_device, Transport_Layer, protocolo usado, timestamp
class Logs(BaseModel):
    ID_device = CharField()
    Transport_Layer = CharField()
    Protocol = CharField()
    timestamp = DateTimeField()

# Definicion modelo Configuracion
# ID_protocol, Transport_Layer <- Deben poder ser cambiables
class Configuracion(BaseModel):
    ID_protocol = CharField()
    Transport_Layer = CharField()

# Definicion modelo Loss
# timestamp(escritura datos - envio de datos), packet_loss
class Loss(BaseModel):
    comm_timestamp = IntegerField()
    packet_loss = CharField()


## Ver la documentación de peewee para más información, es super parecido a Django