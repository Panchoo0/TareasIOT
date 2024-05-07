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
    MAC_device = CharField()
    Transport_layer = CharField()
    ID_protocol = IntegerField()
    Length = IntegerField()
    # All data
    Batt_level = IntegerField()
    timestamp = DateTimeField()
    temp = IntegerField()
    press = CharField()
    hum = IntegerField()
    co = CharField()
    # acc_x = 
    # acc_y =
    # acc_z =
    # rgyr_x =
    # rgyr_y =
    # rgyr_z =
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

    def set_protocol(self, new_ID):
        # config = Configuracion.
        self.ID_protocol = new_ID
        self.save()
    
    def set_Transport_layer(self, new_Transport_layer):
        self.Transport_layer = new_Transport_layer
        
        
# Configuracion.get_by_id(1).set_protocol(2)
        

# Definicion modelo Loss
# timestamp(escritura datos - envio de datos), packet_loss
class Loss(BaseModel):
    comm_timestamp = IntegerField()
    packet_loss = CharField()


## Ver la documentación de peewee para más información, es super parecido a Django