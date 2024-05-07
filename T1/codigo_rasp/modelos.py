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
    # Headers
    ID_message = CharField()
    # MAC_device = 
    # Transport_layer =
    # ID_protocol =
    # Length =  
    # # Data
    # Batt_level = IntegerField()
    # timestamp =
    # temp
    # press 
    # hum 
    # co 
    # acc_x 
    # acc_y 
    # acc_z 
    # rgyr_x 
    # rgyr_y 
    # rgyr_z
    # Extra
    ID_device = CharField()
    MAC = CharField()
    timestamp = DateTimeField()
    


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

    def __init__(self, ID_protocol, Transport_layer, *args, **kwargs) -> None:
        self.ID_protocol = ID_protocol
        self.Transport_Layer = Transport_layer

    def set_protocol(new_ID):
        ID_protocol = new_ID
        

# Definicion modelo Loss
# timestamp(escritura datos - envio de datos), packet_loss
class Loss(BaseModel):
    comm_timestamp = IntegerField()
    packet_loss = CharField()


## Ver la documentación de peewee para más información, es super parecido a Django