import asyncio
import websockets

from config.settings import WEBSOCKET_HOST, WEBSOCKET_PORT
from network.data_receive import DataReceive


class WebSocketServer:

    def __init__(self, audio_manager):

        self.host = WEBSOCKET_HOST
        self.port = WEBSOCKET_PORT

        # DataReceive will later pass audio to AudioManager
        self.data_receiver = DataReceive(audio_manager)

    async def client_handler(self, websocket):

        print("[WS] Client Connected")

        try:

            async for message in websocket:

                # Forward every received message
                await self.data_receiver.receive(
                    websocket,
                    message
                )

        except websockets.exceptions.ConnectionClosed:

            print("[WS] Client Disconnected")

        except Exception as error:

            print(f"[WS] Error : {error}")

    async def start(self):

        print(f"[WS] Starting Server : {self.host}:{self.port}")

        async with websockets.serve(
            self.client_handler,
            self.host,
            self.port,
            max_size=None
        ):

            print("[WS] Server Running")

            await asyncio.Future()