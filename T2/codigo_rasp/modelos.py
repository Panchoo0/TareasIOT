from peewee import Model, PostgresqlDatabase, DateTimeField, CharField, IntegerField, FloatField
#from playhouse.postgres_ext import ArrayField

# Configuración de la base de datos
db_config = {
    'host': 'localhost', 
    'port': 5432, 
    'user': 'postgres', 
    'password': 'postgres', 
    'database': 'iot_db'
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
    Length = IntegerField()
    # All data
    Batt_level = IntegerField()
    temp = IntegerField(null=True)
    press = IntegerField(null=True)
    hum = IntegerField(null=True)
    co = FloatField(null=True)
    RMS = FloatField(null=True)
    Amp_x = FloatField(null=True)
    Frec_x = FloatField(null=True)
    Amp_y = FloatField(null=True)
    Frec_y = FloatField(null=True)
    Amp_z = FloatField(null=True)
    Frec_z = FloatField(null=True)
    
# Definicion modelo Configuracion
class Configuracion(BaseModel):
    Status = IntegerField()
    ID_protocol = IntegerField()
    BMI270_sampling = IntegerField()
    BMI270_Acc_sensibility = IntegerField()
    BMI270_Gyro_sensibility = IntegerField()
    BME688_sampling = IntegerField()
    Discontinous_Time = IntegerField()
    Port_TCP = IntegerField()
    Port_UDP = IntegerField()
    Host_IP = CharField()
    SSID = CharField()
    Password = CharField()

    # def get_ID_protocol(self):
    #     return self.ID_protocol

    # def get_s(self):
    #     return self.Transport_Layer
    
    # def set_protocol(new_ID):
    #     config = Configuracion.get_by_id(1)
    #     config.ID_protocol = new_ID
    #     config.save()
    
    # def set_Transport_layer(new_Transport_layer):
    #     config = Configuracion.get_by_id(1)
    #     print("Cambiando Transport Layer por", new_Transport_layer)
    #     config.Transport_Layer = new_Transport_layer
    #     config.save()

    
        
        
# Configuracion.get_by_id(1).set_protocol(2)


## Ver la documentación de peewee para más información, es super parecido a Django

def create_tables():
    with db:
        db.create_tables([Datos, Configuracion])
        Configuracion.create(ID_protocol='0', Transport_Layer='TCP')

create_tables()