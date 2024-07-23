import socket 
import csv 
import threading
import time 

buffer = []
buffer_lock = threading.Lock()

def write_buffer_to_csv():
    while True:
        print(f"Writing data to csv file each 5 sec...")
        time.sleep(5)
        with buffer_lock: # call the thread locker. 
            if buffer:
                with open("keystrokeData.csv", 'a', newline='') as csvfile:
                    csvwriter = csv.writer(csvfile)
                    csvwriter.writerows(buffer)
                buffer.clear()

flushing_tread = threading.Thread(target=write_buffer_to_csv) # creating a thread object. 
flushing_tread.daemon = True # Make the flushing thread stop when the main thread stops 
flushing_tread.start() # start the write_buffer_to_csv() function in another thread. 


# --- SERVER SETUP --- # 

# Address family: AF.INET -> IPv4
# Socket type: SOCK_STREAM -> (TCP) Sequenced, reliable, two-way connection-based byte streams

# Creates a new socket object   
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Allow to reuse the address & enable TCP keepalive to detect broken connections (might delete later). 
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)


# Network interface: '0.0.0.0' -> All network interfaces the device uses 
# Port: 54000 within each network interface

# Spesify network interface and which port we want to listen from. 
server_socket.bind(('0.0.0.0', 54000))

# Number of connections the server can handle at once 
server_socket.listen(5)
print("Server listening on port 54000")

with open('keystrokeData.csv', 'a', newline='') as csvfile:
    csvwriter = csv.writer(csvfile)

    # Running server and waiting for a connection.
    while True:
    
        client_socket, client_addr = server_socket.accept()
        print(f"Connection from {client_addr}")

        # Listen for data from client. 
        while True:
            try: 
                data = client_socket.recv(4096)
                if not data:
                    print("No data received. Currently listening...")
                    break
                decoded_data = data.decode('utf-8')
                print(f"Received data: {decoded_data} \n")
                
                # Each element in rows is one data and one keypress 
                rows = decoded_data.split('\n')
                
                # lock this thread: 
                with buffer_lock:
                    for row in rows:
                        if row:
                            # The row element is split into two "columns" based on , 
                            buffer.append(row.split(','))
            except ConnectionResetError:
                print(f"Connection reset by peer. Waiting for new connection...")
                break
            except Exception as e:
                print(f"Error has occurred: {e}")
                break

        # Breaking for inner loop -> close the connection to that client
        client_socket.close()
        


