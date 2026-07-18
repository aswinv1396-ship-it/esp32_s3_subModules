import asyncio
import websockets


async def handler(websocket):
    print("Client connected")

    async for message in websocket:
        print("Received:", message)


async def main():
    async with websockets.serve(
        handler,
        "localhost",
        8080
    ):
        print("WebSocket server running")
        await asyncio.Future()


asyncio.run(main())
