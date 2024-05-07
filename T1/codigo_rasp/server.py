import socket
import struct
HOST = '0.0.0.0'  # Escucha en todas las interfaces disponibles
PORT = 1234       # Puerto en el que se escucha
PORT_UDP = 1235

# Crea un socket para IPv4 y conexión TCP
socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

socketTCP.bind((HOST, PORT))
socketUDP.bind((HOST, PORT_UDP))

socketTCP.listen()



# with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
#     s.bind((HOST, PORT))
#     s.listen()

#     print("El servidor está esperando conexiones en el puerto", PORT)

#     while True:
#         conn, addr = s.accept()  # Espera una conexión
#         with conn:
#             print('Conectado por', addr)
#             data = conn.recv(1024)  # Recibe hasta 1024 bytes del cliente
#             if data:
#                 print("Recibido: ", data.decode('utf-8'))
#                 respuesta = "tu mensaje es: " + data.decode('utf-8')
#                 conn.sendall(respuesta.encode('utf-8'))  # Envía la respuesta al cliente

def parse_headers(data):
    if len(data) < 12:
        return None
    
    id = struct.unpack('<h', data[:2])[0]
    mac = struct.unpack('<6s', data[2:8])[0].decode('utf-8')

    Transport_layer = struct.unpack('<c', data[8:9])[0]
    ID_Protocol = struct.unpack('<c', data[9:10])[0]

    msg_len = struct.unpack('<H', data[10:12])[0]

    return {
        'id': id,
        'mac': mac,
        'Transport_layer': Transport_layer,
        'ID_Protocol': ID_Protocol,
        'msg_len': msg_len
    }

def parse_protocol_0(data):
    headers = parse_headers(data)

    batt_lvl = struct.unpack('<B', data[12:13])[0]

    return {
        **headers,
        'batt_lvl': batt_lvl
    }


def parse_data(data):
    protocol = struct.unpack('<c', data[8:9])[0]
    print(data[8:9])
    print(protocol)
    if protocol == b'\x00':
        return parse_protocol_0(data)
    else:
        return None


def add_data(data):

    return

def change_protocol(data):
    
    return

def get_transport_layer(data):
    return


def main():
    conn, addr = socketTCP.accept()  # Espera una conexión del microcontrolador
    ID_protocol, Transport_Layer = (0, "TCP") # Aquí se debe hacer la consulta a la base de datos
    coded_message = f"{ID_protocol}:{Transport_Layer}" # Se le envia al microcontrolador el protocolo y el tipo de transporte
    conn.sendall(coded_message.encode('utf-8'))

    if Transport_Layer == "TCP":
        data = conn.recv(1024)  # Recibe hasta 1024 bytes del cliente
        print("Recibido: ", data)
        print(parse_data(data))
        conn.close()

    elif Transport_Layer == "UDP":
        data, addr = socketUDP.recvfrom(1024)
        print("Recibido: ", data)

while True:
    try:
        main()
    except:
        socketTCP.close()
        socketUDP.close()
        break
