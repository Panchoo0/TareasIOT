import threading
import socket
import struct
from pynput.mouse import Button, Controller
from pynput import keyboard
# from modelos import Datos, Configuracion, Loss
import datetime

HOST = '0.0.0.0'  # Escucha en todas las interfaces disponibles
PORT = 1234       # Puerto en el que se escucha
PORT_UDP = 1235
MAX_SIZE = 1024 * 1024 * 100

def parse_headers(data):
    if len(data) < 4:
        return None
    
    id_protocol = struct.unpack('<B', data[0:1])[0]
    status = struct.unpack('<B', data[1:2])[0]
    len_data = struct.unpack('<H', data[2:4])[0]

    return {
        'ID_Protocol': id_protocol,
        'status': status,
        'msg_len': len_data,
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




def parse_protocol_1(data):
    headers = parse_headers(data)
    batt_lvl = struct.unpack('<B', data[4:5])[0]
    timestamp = datetime.datetime.now()
    print("batt_lvl", batt_lvl)
    print("timestamp", timestamp)
    return {
        **headers,
        'batt_lvl': batt_lvl,
        'timestamp': timestamp
    }


def parse_protocol_2(data):
    protocol_1 = parse_protocol_1(data)
    temp = struct.unpack('<B', data[9:10])[0]
    print("temp", temp)
    press = parse_int(data[10:14])
    print("press", press)

    hum = struct.unpack('<B', data[14:15])[0]
    print("hum", hum)

    co = parse_float(data[15:19])
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
    rms = parse_float(data[19:23])
    print("rms", rms)

    return {
        **protocol_2,
        'rms': rms,
        
    }

def parse_protocol_4(data):
    protocol_3 = parse_protocol_3(data)
    amp_x = parse_float(data[23:27])
    freq_x = parse_float(data[27:31])
    amp_y = parse_float(data[31:35])
    freq_y = parse_float(data[35:39])
    amp_z = parse_float(data[39:43])
    freq_z = parse_float(data[43:47])
    
    return {
        **protocol_3,
        'amp_x': amp_x,
        'freq_x': freq_x,
        'amp_y': amp_y,
        'freq_y': freq_y,
        'amp_z': amp_z,
        'freq_z': freq_z
    }


def parse_protocol_5(data):
    parse_data = parse_protocol_2(data)
    print("Se recibieron", len(data), "bytes")
    acc_x = [struct.unpack('>f', data[19 + i*4:19 + (i + 1)*4])[0]
             for i in range(2000)]
    acc_y = [struct.unpack(
        '>f', data[19 + 8000 + i*4:19 + 8000 + (i + 1)*4])[0] for i in range(2000)]
    acc_z = [struct.unpack(
        '>f', data[19 + 2*8000 + i*4:19 + 2*8000 + (i + 1)*4])[0] for i in range(2000)]
    rgyr_x = [struct.unpack(
        '>f', data[19 + 3*8000 + i*4:19 + 3*8000 + (i + 1)*4])[0] for i in range(2000)]
    rgyr_y = [struct.unpack(
        '>f', data[19 + 4*8000 + i*4:19 + 4*8000 + (i + 1)*4])[0] for i in range(2000)]
    rgyr_z = [struct.unpack(
        '>f', data[19 + 5*8000 + i*4:19 + 5*8000 + (i + 1)*4])[0] for i in range(2000)]

    return {
        **parse_data,
        'acc_x': acc_x,
        'acc_y': acc_y,
        'acc_z': acc_z,
        'rgyr_x': rgyr_x,
        'rgyr_y': rgyr_y,
        'rgyr_z': rgyr_z
    }


def parse_data(data):
    try:
        protocol = struct.unpack('<c', data[1:2])[0]
        if protocol == b'\x01':
            parse_data = parse_protocol_1(data)
        elif protocol == b'\x02':
            parse_data = parse_protocol_2(data)
        elif protocol == b'\x03':
            parse_data = parse_protocol_3(data)
        elif protocol == b'\x04':
            parse_data = parse_protocol_4(data)
        elif protocol == b'\x05':
            parse_data = parse_protocol_5(data)

        if not parse_data:
            return None
        
    except Exception as e:
        print(e)
        return None

class UDP_Server:
    def __init__(self):
        self.stop_event = threading.Event()

    def stop_server(self):
        self.stop_event.set()
        print("Server stopped")

    def udp_server(self):
        socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        socketUDP.bind((HOST, PORT_UDP))

        while True:
            try:
                print("UDP esperando conexión...")
                data, addr = socketUDP.recvfrom(MAX_SIZE)  # Recibe hasta 1024 bytes del cliente
                print("Recibido (UDP)")
                parse_data(data)
                if self.stop_event.is_set():
                    print("Cerrando conexión")
                    msg = 'STOP'
                    socketUDP.sendto(msg.encode('utf-8'), addr)
                    break
            except KeyboardInterrupt:
                print("Cerrando conexión")
                break
            except Exception as e:
                print(e)
                break

        socketUDP.close()
        print("SOCKET stopped")

class TCP_Server:
    def __init__(self):
        self.stop_event = threading.Event()

    def stop_server(self):
        self.stop_event.set()
        print("Server stopped")


    def tcp_server(self):
        socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socketTCP.bind((HOST, PORT))
        socketTCP.listen()

        while True:
            try:
                print("TCP esperando conexión...")
                conn, addr = socketTCP.accept()  # Espera una conexión del microcontrolador
                print("Conexión establecida con", addr, "\n")

                data = conn.recv(MAX_SIZE)  # Recibe hasta 1024 bytes del cliente
                print("Recibido (TCP)")
                parsed_headers = parse_headers(data)
                if parsed_headers['ID_Protocol'] == 4:
                    parte = 2
                    while parsed_headers['msg_len'] != len(data):
                        data += conn.recv(MAX_SIZE)
                        print("Recibido (TCP) parte", parte)
                        print("Data len", len(data))
                        parte += 1

                parse_data(data)
                if self.stop_event.is_set():
                    print("Cerrando conexión")
                    msg = 'STOP'
                    conn.sendall(msg.encode('utf-8'))
                    conn.close()
                    break
                conn.close()
            except KeyboardInterrupt:
                print("Cerrando conexión")
                conn.close()
                break
            except Exception as e:
                print(e)
                conn.close()
                break

        socketTCP.close()
        print("SOCKET stopped")


def tcp_bd(acc_sampling, acc_sensibility, gyro_sensibility, bme688_sampling, disc_time, tcp_port, udp_port, ip_addr, ssid, password, status, id_protocol):
    socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    socketTCP.bind((HOST, PORT))
    socketTCP.listen()

    conn, addr = socketTCP.accept()  # Espera una conexión del microcontrolador

    print("Enviando configuración al microcontrolador")

    try:
        parsed_data = f"{status};{id_protocol};{acc_sampling};{acc_sensibility};{gyro_sensibility};{bme688_sampling};{disc_time};{tcp_port};{udp_port};{ip_addr};{ssid};{password}"
        conn.sendall(parsed_data.encode())
        print("Configuración enviada")
        conn.close()
    except Exception as e:
        print(e)
        conn.close()
