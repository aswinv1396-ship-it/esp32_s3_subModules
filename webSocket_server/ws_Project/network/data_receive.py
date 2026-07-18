import time

from data.data_processing import DataProcessing


class DataReceive:

    def __init__(self, audio_manager):

        self.processor = DataProcessing(audio_manager)

    async def receive(self, websocket, message):

        # ------------------------------------------
        # Receive Information
        # ------------------------------------------

        receive_time = time.strftime("%Y-%m-%d %H:%M:%S")

        print("\n" + "=" * 60)
        print("[DATA] Message Received")
        print("=" * 60)

        print(f"Time : {receive_time}")
        print(f"Size : {len(message)} bytes")

        # ------------------------------------------
        # TEXT MESSAGE
        # ------------------------------------------

        if isinstance(message, str):

            print(f"[TEXT] {message}")

            return

        # ------------------------------------------
        # BINARY MESSAGE
        # ------------------------------------------

        await self.processor.process(message)