import asyncio
import websockets

SERVER_HOST = "0.0.0.0"
SERVER_PORT = 8080

async def start_server():

    print("[SERVER] Starting WebSocket server..." )
    async with websockets.serve(websocket_handler, SERVER_HOST, SERVER_PORT):
        print( f"[SERVER] Listening on " f"ws://{SERVER_HOST}:{SERVER_PORT}"  )
        print( "[SERVER] Waiting for ESP32 connection..." )

        await asyncio.Future()

# ============================================================
# WEBSOCKET CLIENT HANDLER
# ============================================================

async def websocket_handler(websocket):

    print( "[SERVER] ESP32 connected")

    try:
        async for message in websocket:
            print( "[SERVER] Data received"  )
            if isinstance( message, bytes):
                print( f"[SERVER] Binary data length = "  f"{len(message)} bytes")
            else:
                print( f"[SERVER] Text data = " f"{message}" )

    except websockets.exceptions.ConnectionClosed:
        print( "[SERVER] ESP32 disconnected" )


# ============================================================
# MAIN
# ============================================================

def main():

    print( "============================================" )
    print( "        ESP32 AUDIO WEBSOCKET SERVER" )
    print( "============================================")


    try:
        asyncio.run(start_server() )

    except KeyboardInterrupt:
        print("[SYSTEM] Keyboard interrupt" )


    finally:
        print("[SYSTEM] Server shutdown complete")

# ============================================================
# PROGRAM ENTRY
# ============================================================

if __name__ == "__main__":

    main()
