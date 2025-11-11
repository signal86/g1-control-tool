import time
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

debug = False

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
    s.connect((CONFIG['hostname'], CONFIG['port']))

@atexit.register
def close_socket():
    s.close()
    print("socket closed")

# s.send(b'hi')
# time.sleep(1)
# s.send(b'hello world')

arm_list = [
    "high wave",
    "face wave",
    "shake hand",
    "high five",
    "hug",
    "clap",
    "left kiss",
    "right kiss",
    "two-hand kiss",
    "reject",
    "right hand up",
    "x-ray",
    "hands up",
    "heart",
    "right heart"
]

 #
 #  8 extra_actions = [
 #  9     "shake hand",
 # 10     "high five",
 # 11     "hug",
 # 12     "heart",
 # 13     "right heart",
 # 14     "hands up",
 # 15     "x-ray",
 # 16     "right hand up",
 # 17     "reject",
 # 18 ]

controls = {
    'w': False,
    'a': False,
    's': False,
    'd': False,
    'q': False,
    'e': False,
    'prompt': False
}

async def movement():
    while True:
        if controls['prompt'] == True: continue
        # print("movement func")
        action_occurred = False
        if controls['w'] == True and controls['s'] != True:
            print("move forward")
            s.send(b'forward')
            action_occurred = True
        if controls['s'] == True and controls['w'] != True:
            print("move backwards")
            s.send(b'back')
            action_occurred = True
        if controls['q'] == True and controls['e'] != True:
            print("rotate left")
            s.send(b'rotate left')
            action_occurred = True
        if controls['e'] == True and controls['q'] != True:
            print("rotate right")
            s.send(b'rotate right')
            action_occurred = True
        if controls['a'] == True and controls['d'] != True:
            print("move left")
            s.send(b'left')
            action_occurred = True
        if controls['d'] == True and controls['a'] != True:
            print("move right")
            s.send(b'right')
            action_occurred = True
        await asyncio.sleep(1 if action_occurred else 0.01)

def down(key):
    # print(f"down: {key}")
    if controls['prompt'] == True: return
    if key == keyboard.KeyCode.from_char('w'):
        controls['w'] = True
    if key == keyboard.KeyCode.from_char('s'):
        controls['s'] = True
    if key == keyboard.KeyCode.from_char('a'):
        controls['a'] = True
    if key == keyboard.KeyCode.from_char('d'):
        controls['d'] = True
    if key == keyboard.KeyCode.from_char('q'):
        controls['q'] = True
    if key == keyboard.KeyCode.from_char('e'):
        controls['e'] = True


def up(key):
    # print(f"up: {key}")
    if controls['prompt'] == True: return
    if key == keyboard.KeyCode.from_char('w'):
        controls['w'] = False
    if key == keyboard.KeyCode.from_char('s'):
        controls['s'] = False
    if key == keyboard.KeyCode.from_char('a'):
        controls['a'] = False
    if key == keyboard.KeyCode.from_char('d'):
        controls['d'] = False
    if key == keyboard.KeyCode.from_char('q'):
        controls['q'] = False
    if key == keyboard.KeyCode.from_char('e'):
        controls['e'] = False

    if key == keyboard.KeyCode.from_char('/'):
        controls['prompt'] = True

        # to make sure no controls get locked
        controls['w'] = False
        controls['a'] = False
        controls['s'] = False
        controls['d'] = False
        controls['q'] = False
        controls['e'] = False

        print("\nAvailable Commands\n")
        print(arm_list)
        print("or... \'release\' to release the arm")
        print("or... \'led [r] [g] [b]\' to change visor colors")
        os.system("stty echo")
        try:
            prompt = input("\n> ")
            if prompt[0] == "/": prompt = prompt[1:]
            if prompt in arm_list:
                print(f"Valid command, sending")
                s.send(bytes(f"command {prompt}", encoding='utf-8'))
            elif prompt == 'release':
                print(f"Valid command, sending")
                s.send(b'command release arm')
            elif prompt.split(' ', 1)[0] == 'led' and len(prompt.split(' ')) == 4:
                colorchecker = int(prompt.split(' ')[1])
                colorchecker = int(prompt.split(' ')[2])
                colorchecker = int(prompt.split(' ')[3])
                del colorchecker
                print(f"Valid command, sending")
                s.send(bytes(f"{prompt}", encoding='utf-8'))
            else:
                print(f"Command not understood\nRead prompt: {prompt}")

        except Exception as e:
            print(f"!!! Problem in prompt mode: {e}\nReturning to default mode\n")

        os.system("stty -echo")
        controls['prompt'] = False


def listen():
    # print('got here')
    listener = keyboard.Listener(on_press=down, on_release=up)
    listener.start()

async def main():
    listen()
    await movement()

if __name__ == '__main__':
    asyncio.run(main())
