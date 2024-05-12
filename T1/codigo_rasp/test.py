from modelos import db, Configuracion
# Aquí se debe hacer la consulta a la base de dato
# Configuracion
config = Configuracion.get_by_id(1)
print(config.get_Transport_Layer())
Configuracion.set_Transport_layer("UDP")
print(config.get_Transport_Layer())
