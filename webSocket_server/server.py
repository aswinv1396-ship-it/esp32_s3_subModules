import asyncio
import websockets


clients = set()


async def handler(ws):

    print("Client connected")

    clients.add(ws)

    try:

        async for msg in ws:

            print("Received:", msg)


            # broadcast message

            for client in clients:

                if client != ws:
                    await client.send(msg)


    except Exception as e:

        print(e)


    finally:

        clients.remove(ws)

        print("Client disconnected")



async def main():

    server = await websockets.serve(
        handler,
        "0.0.0.0",
        8080
    )

    print("WebSocket server running")
    print("Port: 8080")


    await server.wait_closed()



asyncio.run(main())
