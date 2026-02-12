import time
import atexit
import socket
import os
import asyncio
import threading
import tkinter as tk
import TKinterModernThemes as TKMT

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
    "right heart",
    "release"
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
    'e': False#,
    # 'prompt': False
}

async def movement():
    while True:
        # if controls['prompt'] == True: continue
        # print("movement func")
        # action_occurred = False
        motion_set = []
        if 'w' in keys_down and 's' not in keys_down:
            print("move forward")
            # s.send(b'forward')
            motion_set.append('forward')
            # action_occurred = True
        if 's' in keys_down and 'w' not in keys_down:
            print("move backwards")
            # s.send(b'back')
            motion_set.append('back')
            # action_occurred = True
        if 'q' in keys_down and 'e' not in keys_down:
            print("rotate left")
            # s.send(b'rotate left')
            motion_set.append('rotleft')
            # action_occurred = True
        if 'e' in keys_down and 'q' not in keys_down:
            print("rotate right")
            # s.send(b'rotate right')
            motion_set.append('rotright')
            # action_occurred = True
        if 'a' in keys_down and 'd' not in keys_down:
            print("move left")
            # s.send(b'left')
            motion_set.append('left')
            # action_occurred = True
        if 'd' in keys_down and 'a' not in keys_down:
            print("move right")
            # s.send(b'right')
            motion_set.append('right')
            # action_occurred = True

        if len(motion_set) > 0:
            bytestr = ""
            for i in motion_set:
                bytestr += i + " "
            bytestr = bytestr.rstrip()
            s.send(bytes(bytestr, encoding='utf-8'))
            print()

        await asyncio.sleep(1 if len(motion_set) > 0 else 0.01)
        
keys_down = set()

def down(key):
    # print(f"down: {key.keysym}")
    keys_down.add(key.keysym)
    if key.keysym == 'q':
        key.widget.winfo_toplevel().app.q.config(foreground="aqua")
    if key.keysym == 'w':
        key.widget.winfo_toplevel().app.w.config(foreground="aqua")
    if key.keysym == 'e':
        key.widget.winfo_toplevel().app.e.config(foreground="aqua")
    if key.keysym == 'a':
        key.widget.winfo_toplevel().app.a.config(foreground="aqua")
    if key.keysym == 's':
        key.widget.winfo_toplevel().app.s.config(foreground="aqua")
    if key.keysym == 'd':
        key.widget.winfo_toplevel().app.d.config(foreground="aqua")
    # print(f"down: {key}")
    # if controls['prompt'] == True: return
    # if key == keyboard.KeyCode.from_char('w'):
    #     controls['w'] = True
    # if key == keyboard.KeyCode.from_char('s'):
    #     controls['s'] = True
    # if key == keyboard.KeyCode.from_char('a'):
    #     controls['a'] = True
    # if key == keyboard.KeyCode.from_char('d'):
    #     controls['d'] = True
    # if key == keyboard.KeyCode.from_char('q'):
    #     controls['q'] = True
    # if key == keyboard.KeyCode.from_char('e'):
    #     controls['e'] = True


def up(key):
    # print(f"up: {key.keysym}")
    keys_down.discard(key.keysym)
    if key.keysym == 'q':
        key.widget.winfo_toplevel().app.q.config(foreground="white")
    if key.keysym == 'w':
        key.widget.winfo_toplevel().app.w.config(foreground="white")
    if key.keysym == 'e':
        key.widget.winfo_toplevel().app.e.config(foreground="white")
    if key.keysym == 'a':
        key.widget.winfo_toplevel().app.a.config(foreground="white")
    if key.keysym == 's':
        key.widget.winfo_toplevel().app.s.config(foreground="white")
    if key.keysym == 'd':
        key.widget.winfo_toplevel().app.d.config(foreground="white")
    # print(f"up: {key}")
    # if controls['prompt'] == True: return
    # if key == keyboard.KeyCode.from_char('w'):
    #     controls['w'] = False
    # if key == keyboard.KeyCode.from_char('s'):
    #     controls['s'] = False
    # if key == keyboard.KeyCode.from_char('a'):
    #     controls['a'] = False
    # if key == keyboard.KeyCode.from_char('d'):
    #     controls['d'] = False
    # if key == keyboard.KeyCode.from_char('q'):
    #     controls['q'] = False
    # if key == keyboard.KeyCode.from_char('e'):
    #     controls['e'] = False

#     if key == keyboard.KeyCode.from_char('/'):
#         controls['prompt'] = True
#
#         # to make sure no controls get locked
#         controls['w'] = False
#         controls['a'] = False
#         controls['s'] = False
#         controls['d'] = False
#         controls['q'] = False
#         controls['e'] = False
#
#         print("\nAvailable Commands\n")
#         print(arm_list)
#         print("or... \'release\' to release the arm")
#         print("or... \'led [r] [g] [b]\' to change visor colors")
#         os.system("stty echo")
#         try:
#             prompt = input("\n> ")
#             if prompt[0] == "/": prompt = prompt[1:]
#             if prompt in arm_list:
#                 print(f"Valid command, sending")
#                 s.send(bytes(f"command {prompt}", encoding='utf-8'))
#             elif prompt == 'release':
#                 print(f"Valid command, sending")
#                 s.send(b'command release arm')
#             elif prompt.split(' ', 1)[0] == 'led' and len(prompt.split(' ')) == 4:
#                 colorchecker = int(prompt.split(' ')[1])
#                 colorchecker = int(prompt.split(' ')[2])
#                 colorchecker = int(prompt.split(' ')[3])
#                 del colorchecker
#                 print(f"Valid command, sending")
#                 s.send(bytes(f"{prompt}", encoding='utf-8'))
#             else:
#                 print(f"Command not understood\nRead prompt: {prompt}")
#
#         except Exception as e:
#             print(f"!!! Problem in prompt mode: {e}\nReturning to default mode\n")
#
#         os.system("stty -echo")
#         controls['prompt'] = False


def command(cmd):
    print(f"sending {cmd}")
    if cmd in arm_list or cmd == "release":
        s.send(bytes(f"command {cmd}", encoding='utf-8'))

old = [0, 0, 0]
def led_change(master):
    global old
    if old[0] != r.get() or old[1] != g.get() or old[2] != b.get():
        s.send(bytes(f"led {r.get()} {g.get()} {b.get()}", encoding='utf-8'))
        old = [r.get(), g.get(), b.get()]
        canvas.config(bg=f"#{r.get():02x}{g.get():02x}{b.get():02x}")
        print(f"led changed: {r.get()} {g.get()} {b.get()}")

    master.after(250, lambda master=master: led_change(master))

class App(TKMT.ThemedTKinterFrame):
    def __init__(self):
        super().__init__("g1 control tool", "park", "dark")
        self.frame = self.addFrame("main")
        self.master.app = self
        self.q = self.frame.Label("Q")
        self.q.grid(row=5, column=0, sticky="w")
        self.w = self.frame.Label("W")
        self.w.grid(row=5, column=1, sticky="w")
        self.e = self.frame.Label("E")
        self.e.grid(row=5, column=2, sticky="w")
        self.a = self.frame.Label("A")
        self.a.grid(row=6, column=0, sticky="w")
        self.s = self.frame.Label("S")
        self.s.grid(row=6, column=1, sticky="w")
        self.d = self.frame.Label("D")
        self.d.grid(row=6, column=2, sticky="w")
        for i, cmd in enumerate(arm_list):
            self.frame.Button(f"{cmd}", command=lambda cmd=cmd: command(cmd)).grid(row=i, column=4, sticky="e")
        self.frame.master.grid_columnconfigure(4, weight=1)
        global r
        global g
        global b
        r = tk.IntVar(value=0)
        g = tk.IntVar(value=0)
        b = tk.IntVar(value=0)
        global canvas
        canvas = tk.Canvas(self.frame.master, width=200, height=50, bg="black")
        canvas.grid(row=1, column=0, sticky="w", columnspan=3, padx=10)
        self.frame.Label("(slide to change visor color, RGB)", size=8).grid(row=0, column=0, columnspan=3, sticky="")
        self.frame.Scale(lower=0, upper=255, variable=r).grid(row=2, column=0, columnspan=3, sticky="w")
        self.frame.Scale(lower=0, upper=255, variable=g).grid(row=3, column=0, columnspan=3, sticky="w")
        self.frame.Scale(lower=0, upper=255, variable=b).grid(row=4, column=0, columnspan=3, sticky="w")
        g.set(255)
        self.master.focus_set()
        self.master.bind("<KeyPress>", down)
        self.master.bind("<KeyRelease>", up)
        led_change(self.frame.master)
        self.run()

def _async(loop):
    asyncio.set_event_loop(loop)
    loop.run_forever()

if __name__ == '__main__':
    # asyncio.create_task(movement())
    loop = asyncio.new_event_loop()
    threading.Thread(target=_async, args=(loop,), daemon=True).start()
    asyncio.run_coroutine_threadsafe(movement(), loop)

    a = App()
