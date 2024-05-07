from peewee import Model, PostgresqlDatabase, DateTimeField, CharField, IntegerField, FloatField
from playhouse.postgres_ext import ArrayField

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
    co = FloatField()
    acc_x = ArrayField(FloatField)
    acc_y = ArrayField(FloatField)
    acc_z = ArrayField(FloatField)
    rgyr_x = ArrayField(FloatField)
    rgyr_y = ArrayField(FloatField)
    rgyr_z = ArrayField(FloatField)
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

    def get_ID_protocol(self):
        return self.ID_protocol

    def get_Transport_layer(self):
        return self.Transport_Layer
    
    def set_protocol(self, new_ID):
        config = Configuracion.get_by_id(1)
        config.ID_protocol = new_ID
        config.save()
    
    def set_Transport_layer(self, new_Transport_layer):
        config = Configuracion.get_by_id(1)
        config.Transport_layer = new_Transport_layer
        config.save()

    
        
        
# Configuracion.get_by_id(1).set_protocol(2)
        

# Definicion modelo Loss
# timestamp(escritura datos - envio de datos), packet_loss
class Loss(BaseModel):
    comm_timestamp = IntegerField()
    packet_loss = CharField()


## Ver la documentación de peewee para más información, es super parecido a Django