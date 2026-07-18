import asyncio
import struct
import websockets

AUDIO_PACKET_MAGIC = 0x41554430
AUDIO_PACKET_VERSION = 1

HEADER_FORMAT = "<IBBHII"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)


async def receive_audio(websocket):

    print("ESP32 client connected")

    try:

        async for message in websocket:

            if isinstance(message, str):

                print("TEXT:", message)

                continue

            data = message

            if len(data) < HEADER_SIZE:

                print(
                    "Packet too small:",
                    len(data)
                )

                continue

            (
                magic,
                version,
                channels,
                sample_count,
                sample_rate,
                sequence_number
            ) = struct.unpack(
                HEADER_FORMAT,
                data[:HEADER_SIZE]
            )

            if magic != AUDIO_PACKET_MAGIC:

                print(
                    "Invalid magic:",
                    hex(magic)
                )

                continue

            if version != AUDIO_PACKET_VERSION:

                print(
                    "Unsupported version:",
                    version
                )

                continue

            expected_audio_bytes = (
                sample_count * 2
            )

            actual_audio_bytes = (
                len(data) - HEADER_SIZE
            )

            if actual_audio_bytes < expected_audio_bytes:

                print(
                    "Incomplete audio packet"
                )

                continue

            audio_data = data[
                HEADER_SIZE:
                HEADER_SIZE + expected_audio_bytes
            ]

            samples = struct.unpack(
                f"<{sample_count}h",
                audio_data
            )

            print(
                f"AUDIO: "
                f"seq={sequence_number}, "
                f"samples={sample_count}, "
                f"rate={sample_rate}, "
                f"channels={channels}, "
                f"first={samples[0]}"
            )

    except websockets.ConnectionClosed:

        print(
            "ESP32 disconnected"
        )


async def main():

    print(
        "WebSocket server listening on port 8080"
    )

    async with websockets.serve(
        receive_audio,
        "0.0.0.0",
        8080,
        max_size=None
    ):

        await asyncio.Future()


if __name__ == "__main__":

    asyncio.run(main())
