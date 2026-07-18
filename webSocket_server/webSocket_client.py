import websocket


ws = websocket.create_connection(
    "wss://preseason-treadmill-aptly.ngrok-free.dev"
)

print("CONNECTED")

ws.send("TEST_FROM_UBUNTU")

print(
    ws.recv()
)
