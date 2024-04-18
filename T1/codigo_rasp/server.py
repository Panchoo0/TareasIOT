import socket

HOST = '0.0.0.0'  # Escucha en todas las interfaces disponibles
PORT = 1234       # Puerto en el que se escucha
PORT_UDP = 1235

conn_type = "UDP"    

# Crea un socket para IPv4 y conexión TCP
socketTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
socketUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

socketTCP.bind((HOST, PORT))
socketUDP.bind((HOST, PORT_UDP))

socketTCP.listen(3)



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


def main():
    if conn_type == "TCP":
        conn, addr = socketTCP.accept()  # Espera una conexión
        data = conn.recv(1024)  # Recibe hasta 1024 bytes del cliente
        print("Recibido: ", data.decode('utf-8'))
        conn.close()

    elif conn_type == "UDP":
        data, addr = socketUDP.recvfrom(1024)
        print("Recibido: ", data.decode('utf-8'))

while True:
    main()
