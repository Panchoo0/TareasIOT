import socket
import struct
from pynput.mouse import Button, Controller
from pynput import keyboard
from modelos import db, Configuracion
HOST = '0.0.0.0'  # Escucha en todas las interfaces disponibles
PORT = 1234       # Puerto en el que se escucha
PORT_UDP = 1235
MAX_SIZE = 1024 * 1024 * 1000

# Crea un socket para IPv4 y conexión TCP
socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

socketTCP.bind((HOST, PORT))
socketUDP.bind((HOST, PORT_UDP))

PRESSED_KEY = ""

def on_press(key):
    global PRESSED_KEY
    if key == keyboard.Key.esc:
        return False  # stop listener
    try:
        k = key.char  # single-char keys
    except:
        k = key.name  # other keys
    if k in ['0', '1', '2', '3', '4', 't', 'u']:  # keys of interest
        print('Key pressed: ' + k)
        PRESSED_KEY = k
        if k == 't':
            Configuracion.set_Transport_layer("TCP")
        elif k == 'u':
            print("UDP")
            Configuracion.set_Transport_layer("UDP")
        else:
            Configuracion.set_protocol(k)
        


listener = keyboard.Listener(on_press=on_press)
listener.start()  # start to listen on a separate thread


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
    print("id", id)
    mac_1 = hex(struct.unpack('<B', data[2:3])[0])[2:]
    mac_2 = hex(struct.unpack('<B', data[3:4])[0])[2:]
    mac_3 = hex(struct.unpack('<B', data[4:5])[0])[2:]
    mac_4 = hex(struct.unpack('<B', data[5:6])[0])[2:]
    mac_5 = hex(struct.unpack('<B', data[6:7])[0])[2:]
    mac_6 = hex(struct.unpack('<B', data[7:8])[0])[2:]

    mac = f"{mac_1}:{mac_2}:{mac_3}:{mac_4}:{mac_5}:{mac_6}"
    print("mac", mac)
    
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

def parse_float(d_bytes):
    f_1 = struct.unpack('<B', d_bytes[0:1])[0]
    f_2 = struct.unpack('<B', d_bytes[1:2])[0]
    f_3 = struct.unpack('<B', d_bytes[2:3])[0]
    f_4 = struct.unpack('<B', d_bytes[3:4])[0]

    f = f_1 << 24 | f_2 << 16 | f_3 << 8 | f_4
    f = f.to_bytes(4, 'little')
    f = struct.unpack('<f', f)[0]

    return f

def parse_int(d_bytes):
    i_1 = struct.unpack('<B', d_bytes[0:1])[0]
    i_2 = struct.unpack('<B', d_bytes[1:2])[0]
    i_3 = struct.unpack('<B', d_bytes[2:3])[0]
    i_4 = struct.unpack('<B', d_bytes[3:4])[0]

    i = i_1 << 24 | i_2 << 16 | i_3 << 8 | i_4

    return i

def parse_protocol_0(data):
    headers = parse_headers(data)
    print("parsed",headers)
    batt_lvl = struct.unpack('<B', data[12:13])[0]
    print("batt_lvl", batt_lvl)

    return {
        **headers,
        'batt_lvl': batt_lvl
    }

def parse_protocol_1(data):
    protocol_0 = parse_protocol_0(data)
    timestamp = data[13:17]
    print("timestamp", timestamp) 
    return {
        **protocol_0,
        'timestamp': timestamp
    }

def parse_protocol_2(data):
    protocol_1 = parse_protocol_1(data)
    temp = struct.unpack('<B', data[17:18])[0]
    print("temp", temp)
    press = parse_int(data[18:22])
    print("press", press)
    print("press 2", struct.unpack('>i', data[18:22])[0])

    hum = struct.unpack('<B', data[22:23])[0]
    print("hum", hum)
    
    co = parse_float(data[23:27])
    print("co", co)

    return {
        **protocol_1,
        'temp': temp,
        'press': press,
        'hum': hum,
        'co': co
    }

def parse_protocol_3(data):
    protocol_2 = parse_protocol_2(data)
    rms = parse_float(data[27:31])
    ampx = parse_float(data[31:35])
    freqx = parse_float(data[35:39])
    ampy = parse_float(data[39:43])
    freqy = parse_float(data[43:47])
    ampz = parse_float(data[47:51])
    freqz = parse_float(data[51:55])

    print("rms", rms)
    print("rms 2", struct.unpack('>f', data[27:31])[0])
    print("ampx", ampx)
    print("freqx", freqx)
    print("ampy", ampy)
    print("freqy", freqy)
    print("ampz", ampz)
    print("freqz", freqz)
    print("ampx as int", struct.unpack('>i', data[31:35])[0])
    

    return {
        **protocol_2,
        'rms': rms,
        'ampx': ampx,
        'ampy': ampy,
        'ampz': ampz,
        'freqx': freqx,
        'freqy': freqy,
        'freqz': freqz
    }

def parse_protocol_4(data):
    parse_data = parse_protocol_2(data)
    print("Se parseo protocolo 2")
    print(len(data))
    print(len(data[27:27 + 8000]))
    acc_x = struct.unpack('>f', data[27:27 + 8000])[0]
    # acc_y = struct.unpack('>f', data[27 + 8000:2*8000 + 27])[0]
    # acc_z = struct.unpack('>f', data[27 + 2*8000:3*8000 + 27])[0]
    # rgyr_x = struct.unpack('>f', data[27 + 3*8000:4*8000 + 27])[0]
    # rgyr_y = struct.unpack('>f', data[27 + 4*8000:5*8000 + 27])[0]
    # rgyr_z = struct.unpack('>f', data[27 + 5*8000:6*8000 + 27])[0]

    print("acc_x", acc_x[:27])

    return {
        **parse_data,
        'acc_x': acc_x,
        # 'acc_y': acc_y,
        # 'acc_z': acc_z,
        # 'rgyr_x': rgyr_x,
        # 'rgyr_y': rgyr_y,
        # 'rgyr_z': rgyr_z
    }


def parse_data(data):
    protocol = struct.unpack('<c', data[9:10])[0]
    if protocol == b'\x00':
        return parse_protocol_0(data)
    elif protocol == b'\x01':
        return parse_protocol_1(data)
    elif protocol == b'\x02':
        return parse_protocol_2(data)
    elif protocol == b'\x03':
        return parse_protocol_3(data)
    elif protocol == b'\x04':
        return parse_protocol_4(data)
    

def add_data(data):
    return


def get_transport_layer(data):
    return


def main():
    conn, addr = socketTCP.accept()  # Espera una conexión del microcontrolador
    ID_protocol = Configuracion.get_by_id(1).get_ID_protocol()
    Transport_Layer = Configuracion.get_by_id(1).get_Transport_layer()
    # ID_protocol, Transport_Layer = (3, "UDP") # Aquí se debe hacer la consulta a la base de datos, también un id para el mensaje
    # ID_protocol2 = Configuracion.get_by_id(1)# Aquí se debe hacer la consulta a la base de datos
    # print('id y layer 2: ',ID_protocol2)
    coded_message = f"{ID_protocol}:{Transport_Layer}" # Se le envia al microcontrolador el protocolo y el tipo de transporte
    conn.sendall(coded_message.encode('utf-8'))
    print("Enviado: ", coded_message)

    if Transport_Layer == "TCP":
        data = conn.recv(MAX_SIZE)  # Recibe hasta 1024 bytes del cliente
        # print("Recibido: ", data)
        data = parse_data(data)

    elif Transport_Layer == "UDP":
        data, addr = socketUDP.recvfrom(MAX_SIZE)
        print("Recibido: ", data)
        data = parse_data(data)
        if PRESSED_KEY == "t":
            socketUDP.settimeout(3)
            socketUDP.sendto("TCP".encode('utf-8'), addr)
            socketUDP.settimeout(None)

    conn.close()




while True:
    try:
        main()
    except Exception as e:
        print(e)
    except KeyboardInterrupt:
        print("Cerrando el servidor...")
        break

socketTCP.close()
socketUDP.close()
