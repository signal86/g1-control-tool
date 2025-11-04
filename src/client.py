debug = True

# import time
import atexit
import socket
import os
import asyncio
os.system("stty -echo")

from pynput import keyboard

# with open(".env", 'r') as f:
#     line = f.readline()
#     while line:
#         line = line.rstrip().split('=', 1)
#         os.environ[line[0]] = line[1]
#         line = f.readline()

CONFIG = {
    "hostname": "192.168.0.66",
    # "username": "unitree",
    # "password": os.getenv("unitree_pw"),
    "port": 10555
}

# ssh_command = [
#     'sshpass', '-p', CONFIG['password'],
#     'ssh',
#     "-o", "ControlMaster=yes",
#     "-o", "ControlPath=/tmp/ssh-%r@%h:%p",
#     "-o", "ControlPersist=600",
#     "-o", "StrictHostKeyChecking=no",
#     f"{CONFIG['username']}@{CONFIG['hostname']}",
#     '-p', str(CONFIG['port']),
#     "echo 'SSH opened'"
# ]

# print(CONFIG["password"])

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
if debug:
    s.connect(('127.0.0.1', CONFIG['port']))
else:
    s.connect((CONFIG['hostname'], CONFIG['PORT']))

@atexit.register
def close_socket():
    s.close()
    print("socket closed")

# s.send(b'hi')
# time.sleep(1)
# s.send(b'hello world')

controls = {
    'w': False,
    'a': False,
    's': False,
    'd': False
}

async def movement():
    while True:
        # print("movement func")
        action_occurred = False
        if controls['w'] == True:
            print("move forward")
            action_occurred = True
        await asyncio.sleep(1 if action_occurred else 0.01)

def down(key):
    # print(f"down: {key}")
    if key == keyboard.KeyCode.from_char('w'):
        controls['w'] = True

def up(key):
    # print(f"up: {key}")
    if key == keyboard.KeyCode.from_char('w'):
        controls['w'] = False

def listen():
    # print('got here')
    listener = keyboard.Listener(on_press=down, on_release=up)
    listener.start()

async def main():
    listen()
    await movement()

if __name__ == '__main__':
    asyncio.run(main())
