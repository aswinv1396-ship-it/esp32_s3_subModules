import asyncio

from network.websocket_server import WebSocketServer
from audio.audio_manager import AudioManager


async def main():
    print("=" * 50)
    print(" Voice Streaming Server")
    print("=" * 50)

    audio_manager = AudioManager()

    websocket_server = WebSocketServer(audio_manager)

    audio_task = asyncio.create_task(
        audio_manager.run()
    )

    websocket_task = asyncio.create_task(
        websocket_server.start()
    )

    print("[SYSTEM] Server Started")

    # -----------------------------------
    # Wait Forever
    # -----------------------------------

    await asyncio.gather(
        websocket_task,
        audio_task
    )


if __name__ == "__main__":
    try:
        asyncio.run(main())

    except KeyboardInterrupt:
        print("\n[SYSTEM] Shutdown Requested")