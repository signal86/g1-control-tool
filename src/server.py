import atexit
import socket

# localhost
# Only have as true if you're running the socket on your local machine
debug = True

CONFIG = {
    # "hostname": "192.168.0.66",
    "port": 10555
}

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
if debug:
    s.bind(('127.0.0.1', CONFIG['port']))
else:
    s.bind(('0.0.0.0', CONFIG['port']))
s.listen(1)

print("wait")
try:
    conn, addr = s.accept()
    print(f"connected: {conn}")

    while True:
        data = conn.recv(1024)
        print(f"received: {data.decode()}")

        if not data: break

    conn.close()

except socket.error as e:
    print(f"error: {e}")
    s.close()

s.close()
