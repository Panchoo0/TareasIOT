from modelos import db, Configuracion
# Aquí se debe hacer la consulta a la base de dato
# Configuracion
ID_protocol2 = Configuracion.get_by_id(1)
print(ID_protocol2.get_Transport_layer())
