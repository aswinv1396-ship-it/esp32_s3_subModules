import websocket


ws = websocket.WebSocket()

ws.connect(
    "ws://192.168.12.230:8080"
)


print("Connected")


ws.send(
    "HELLO_FROM_UBUNTU"
)


while True:

    data = ws.recv()

    print(
        "RX:",
        data
    )
