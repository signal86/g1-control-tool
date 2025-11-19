import atexit
import socket

# localhost
# Only have as true if you're running the socket on your local machine
debug = False

if not debug: 
    from unitree_sdk2py.core.channel import ChannelSubscriber, ChannelFactoryInitialize
    from unitree_sdk2py.idl.default import unitree_go_msg_dds__SportModeState_
    from unitree_sdk2py.idl.unitree_go.msg.dds_ import SportModeState_
    from unitree_sdk2py.g1.loco.g1_loco_client import LocoClient
    from unitree_sdk2py.g1.arm.g1_arm_action_client import G1ArmActionClient
    from unitree_sdk2py.g1.arm.g1_arm_action_client import action_map
    from unitree_sdk2py.g1.audio.g1_audio_client import AudioClient

    ChannelFactoryInitialize()

    motion_client = LocoClient()
    motion_client.SetTimeout(10.0)
    motion_client.Init()

    arm_client = G1ArmActionClient()
    arm_client.SetTimeout(10.0)
    arm_client.Init()

    audio_client = AudioClient()
    audio_client.SetTimeout(10.0)
    audio_client.Init()

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
    while True:
        conn, addr = s.accept()
        print(f"connected: {conn}")

        try:
            while True:
                data = conn.recv(1024)
                print(f"received: {data.decode()}")

                x = 0
                y = 0
                z = 0

                if data.decode().startswith("command "):
                    d = data.decode().split('command ', 1)
                    command = d[1]
                    if not debug: arm_client.ExecuteAction(action_map.get(command))
                    print("Command executed:", command)
                    continue

                if data.decode().split(' ')[0] == "led":
                    colors = data.decode().split(' ', 1)[1].split(' ')
                    if not debug: audio_client.LedControl(int(colors[0]), int(colors[1]), int(colors[2]))
                    print("Colors changed:", colors)
                    continue

                data = data.decode().split()
                for movement in data:
                    if movement == 'forward':
                        print("Moving forward")
                        x = 0.25

                    if movement == 'back':
                        print("Moving backwards")
                        x = -0.25

                    if movement == 'left':
                        print("Moving left")
                        y = 0.25

                    if movement == 'right':
                        print("Moving right")
                        y = -0.25

                    if movement == 'rotleft':
                        print("Rotating left")
                        z = 0.5

                    if movement == 'rotright':
                        print("Rotating right")
                        z = -0.5

                if not debug: motion_client.Move(x, y, z)

                if not data: break

        finally:
            conn.close()

except socket.error as e:
    print(f"error: {e}")
    s.close()

s.close()
