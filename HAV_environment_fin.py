import numpy as np
import socket
import struct
import zlib
import time

# 
# ЧАСТОТА СЛЕДОВАНИЯ ПАКЕТОВ - 20Гц
#
port = 36215
address = '127.0.0.1'
''' 
**** Структура информационного пакета ****
**** Byteorder: big-endian            ****
--- HEAD
 [0x04]
 [0x5C]         
 [DATA PACKETS COUNT]   [uchar]
--- DATA
    -- DATA PACKET #0 --
 [ID]           [int]   [4 bytes]
 [speed]        [float] [4 bytes]
 [X coord rel.] [float] [4 bytes]
 [Y coord rel.] [float] [4 bytes]
 [W]            [float] [4 bytes]
 [L]            [float] [4 bytes]
.
.
.
    -- DATA PACKET #N --
 [ID]           [int]   [4 bytes]
 [speed]        [float] [4 bytes]
 [X coord rel.] [float] [4 bytes]
 [Y coord rel.] [float] [4 bytes]
 [W]            [float] [4 bytes]
 [L]            [float] [4 bytes]
--- TAIL
 [CRC32]*       [int]   [4 bytes] 
 [0xA0]
 [0x5C]
 ---

ID              - Идентификационный номер обнаруженного ТС
speed           - Скорость ТС [м/с]
Y, X coord rel. - Координаты ТС, относительно нашего ТС [м]
W, L            - Габариты ТС [м]

 * Контрольная сумма считается, начиная с байта c индексом 2 информационного сообщения.

'''
ids_array       = np.array([0,      1,      2,      3,      4,      5,      6,      7,      8,      9,      10,     11], dtype=int)
speeds_array    = np.array([16.7,   19.44,  16.9,   16.6,   16.7,   16.65,  16.6,   26.22,  19.44,  25.7,   19.6,   17.8], dtype=float)
X_array         = np.array([0.,     -3.,    0.,     3.,     3.2,     3.,    0.,    -3.1,    -3.2,   -2.9,   -2.8,   -3.1], dtype=float)
Y_array         = np.array([0.,     12.2,   18.2,   14.2,   0.,     -16.2,  -11.6,  -32.2,  0.,     -55.3,  -67.9,  -80.1], dtype=float)
W = 1.8
L = 4.4

ticks = 800
noise = np.random.normal(0, 0.25, ticks * 3)
visible_vehicles = np.uint8(0)
message = bytearray()

def get_rand_noise():
    return noise[np.random.randint(0, ticks * 3 - 1)]


with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

    print("Hello world!")

    s.bind((address, port))
    s.listen()
    conn, addr = s.accept()

    with conn:

        for i in range(ticks):

            visible_vehicles = 0

            message.clear()
            message.append(0x04)
            message.append(0x5c)
            message.append(0x00)
            
            for v in range(11):
                
                if v == 7 or v == 9:
                    a = 0
                    b = 0
                    if(v == 7) :
                        a = 8
                        b = 7
                    elif v == 9 :
                        a = 7
                        b = 9

                    if ((Y_array[a] - Y_array[b]) < (speeds_array[a])) and (speeds_array[b] > speeds_array[a]):
                        speeds_array[v] -= 0.12
                
                Y_array[v + 1] += ((speeds_array[v + 1] + get_rand_noise()) - (speeds_array[0] + get_rand_noise())) * 0.05
                x_n = get_rand_noise()

                if (abs(Y_array[v]) - L * 0.5) < 25 and (abs(X_array[v]) - W * 0.5) < 25:
                    visible_vehicles += 1
                    mBytes = ids_array[v].item().to_bytes(4, byteorder='big')
                    message.append(mBytes[0])
                    message.append(mBytes[1])
                    message.append(mBytes[2])
                    message.append(mBytes[3])
                    mBytes = bytearray(struct.pack("f", speeds_array[v]))
                    message.append(mBytes[0])
                    message.append(mBytes[1])
                    message.append(mBytes[2])
                    message.append(mBytes[3])
                    mBytes = bytearray(struct.pack("f", X_array[v]))
                    message.append(mBytes[0])
                    message.append(mBytes[1])
                    message.append(mBytes[2])
                    message.append(mBytes[3])
                    mBytes = bytearray(struct.pack("f", Y_array[v]))
                    message.append(mBytes[0])
                    message.append(mBytes[1])
                    message.append(mBytes[2])
                    message.append(mBytes[3])
                    mBytes = bytearray(struct.pack("f", W))
                    message.append(mBytes[0])
                    message.append(mBytes[1])
                    message.append(mBytes[2])
                    message.append(mBytes[3])
                    mBytes = bytearray(struct.pack("f", L))
                    message.append(mBytes[0])
                    message.append(mBytes[1])
                    message.append(mBytes[2])
                    message.append(mBytes[3])
   
                pass

            
            message[2] = visible_vehicles

            crc32 = zlib.crc32(message[2:(len(message))])
            mBytes = crc32.to_bytes(4, byteorder='big')

            print("crc:", crc32, "crc bytes:", mBytes[0], mBytes[1], mBytes[2], mBytes[3])

            message.append(mBytes[0])
            message.append(mBytes[1])
            message.append(mBytes[2])
            message.append(mBytes[3])
            message.append(0xA0)
            message.append(0x5C)

            conn.sendall(message)
            
            time.sleep(0.05)
            
            pass

        conn.close()
        s.close()