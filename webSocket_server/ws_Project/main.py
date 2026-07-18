import asyncio

from network.websocket_server import WebSocketServer
from audio.audio_manager import AudioManager



async def main():

    print("=" * 60)
    print(" ESP32 Voice Streaming Server")
    print("=" * 60)


    # -------------------------------------------------
    # Initialize Audio System
    # -------------------------------------------------

    audio_manager = AudioManager()



    # -------------------------------------------------
    # Initialize WebSocket Server
    # -------------------------------------------------

    websocket_server = WebSocketServer(
        audio_manager
    )



    # -------------------------------------------------
    # Start Background Services
    # -------------------------------------------------

    audio_task = asyncio.create_task(

        audio_manager.run()

    )


    websocket_task = asyncio.create_task(

        websocket_server.start()

    )



    print(
        "[SYSTEM] Server Started"
    )



    # -------------------------------------------------
    # Run Forever
    # -------------------------------------------------

    try:

        await asyncio.gather(

            websocket_task,

            audio_task

        )


    except asyncio.CancelledError:

        print(
            "[SYSTEM] Tasks Cancelled"
        )



    finally:

        print(
            "[SYSTEM] Shutdown"
        )



# -------------------------------------------------
# ENTRY POINT
# -------------------------------------------------

if __name__ == "__main__":


    try:

        asyncio.run(
            main()
        )


    except KeyboardInterrupt:


        print(
            "\n[SYSTEM] Keyboard Interrupt"
        )