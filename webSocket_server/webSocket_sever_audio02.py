import asyncio
import websockets
import struct
import numpy as np
import sounddevice as sd
import wave
import os
import time

# ---------------------------------------------------------
# AUDIO PACKET FORMAT
# ---------------------------------------------------------

AUDIO_PACKET_MAGIC = 0x41554430       # "AUD0"
AUDIO_PACKET_VERSION = 1

AUDIO_SAMPLE_RATE = 16000
AUDIO_CHANNELS = 1
AUDIO_BITS_PER_SAMPLE = 16
AUDIO_SAMPLES_PER_PACKET = 512

HEADER_FORMAT = "<IBBHII"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

print("Header size:", HEADER_SIZE)


# ============================================================
# FILE CONFIGURATION
# ============================================================

AUDIO_SAVE_DIRECTORY = "received_audio"

os.makedirs( AUDIO_SAVE_DIRECTORY, exist_ok=True)

# ---------------------------------------------------------
# AUDIO PLAYBACK
# ---------------------------------------------------------

audio_stream = sd.RawOutputStream(
    samplerate=AUDIO_SAMPLE_RATE,
    channels=AUDIO_CHANNELS,
    dtype="int16",
    blocksize=AUDIO_SAMPLES_PER_PACKET
)

audio_stream.start()

print("Audio playback started")

# ============================================================
# AUDIO FILE STATE
# ============================================================

audio_file = None

audio_file_name = None

audio_file_start_time = None


# ============================================================
# START AUDIO FILE
# ============================================================

def start_audio_recording():

    global audio_file
    global audio_file_name
    global audio_file_start_time


    timestamp = time.strftime(
        "%Y%m%d_%H%M%S"
    )


    audio_file_name = os.path.join(

        AUDIO_SAVE_DIRECTORY,

        f"audio_{timestamp}.wav"
    )


    audio_file = wave.open(

        audio_file_name,

        "wb"
    )


    audio_file.setnchannels(

        AUDIO_CHANNELS
    )


    audio_file.setsampwidth(

        AUDIO_BITS_PER_SAMPLE // 8
    )


    audio_file.setframerate(

        AUDIO_SAMPLE_RATE
    )


    audio_file_start_time = time.time()


    print(
       "[RECORDING] Started:",  audio_file_name  )

# ============================================================
# SAVE RECEIVED AUDIO
# ============================================================

def save_audio_data(audio_data):

    global audio_file


    if audio_file is None:

        start_audio_recording()


    audio_file.writeframes(

        audio_data
    )


# ============================================================
# STOP AUDIO RECORDING
# ============================================================

def stop_audio_recording():

    global audio_file


    if audio_file is not None:

        audio_file.close()


        print(

            "[RECORDING] Saved:",

            audio_file_name
        )


        audio_file = None


# ============================================================
# AUDIO TO TEXT
# ============================================================

def audio_to_text(audio_file_path):

    print(

        "[STT] Processing:",

        audio_file_path
    )


    try:

        import speech_recognition as sr


        recognizer = sr.Recognizer()


        with sr.AudioFile(

            audio_file_path

        ) as source:


            audio = recognizer.record(

                source
            )


        text = recognizer.recognize_google(

            audio
        )


        print(

            "[STT] TEXT:",

            text
        )


        return text


    except Exception as error:

        print(

            "[STT] Error:",

            error
        )


        return None



# ---------------------------------------------------------
# WEBSOCKET CLIENT HANDLER
# ---------------------------------------------------------

async def websocket_handler(websocket):

    print("ESP32 connected")

    try:

        async for message in websocket:

            # -------------------------------------------------
            # TEXT MESSAGE
            # -------------------------------------------------

            if isinstance(message, str):

                print("TEXT:", message)

                continue


            # -------------------------------------------------
            # BINARY AUDIO MESSAGE
            # -------------------------------------------------

            data = message

            if len(data) < HEADER_SIZE:

                print("Invalid packet: too small")

                continue


            # -------------------------------------------------
            # UNPACK HEADER
            # -------------------------------------------------

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


            # -------------------------------------------------
            # VALIDATE PACKET
            # -------------------------------------------------

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
            
            # ------------------------------------------------
            # VALIDATE AUDIO PARAMETERS
            # ------------------------------------------------

            if channels != AUDIO_CHANNELS:


                print(

                    "[ERROR] Invalid channel count:",

                    channels
                )


                continue


            if sample_rate != AUDIO_SAMPLE_RATE:


                print(

                    "[ERROR] Invalid sample rate:",

                    sample_rate
                )


                continue

            # -------------------------------------------------
            # EXTRACT PCM AUDIO DATA
            # -------------------------------------------------

            audio_data = data[HEADER_SIZE:]


            expected_size = sample_count * 2


            if len(audio_data) != expected_size:

                print(
                    "Invalid audio size:",
                    len(audio_data),
                    "expected:",
                    expected_size
                )

                continue

            # ------------------------------------------------
            # PLAY AUDIO
            # ------------------------------------------------

            audio_stream.write(

                audio_data
            )


            # ------------------------------------------------
            # SAVE AUDIO
            # ------------------------------------------------

            save_audio_data(

                audio_data
            )


            # -------------------------------------------------
            # PRINT DEBUG INFORMATION
            # -------------------------------------------------

            first_sample = struct.unpack_from(
                "<h",
                audio_data,
                0
            )[0]


            # -------------------------------------------------
            # PLAY AUDIO
            # -------------------------------------------------

            audio_stream.write(audio_data)


    except websockets.exceptions.ConnectionClosed:
        print("ESP32 disconnected")


    except Exception as e:
        print("WebSocket error:", e)


# ---------------------------------------------------------
# SERVER MAIN
# ---------------------------------------------------------

async def main():

    print("Starting WebSocket server...")

    audio_stream.start()

    print( "[AUDIO] Playback started" )


    async with websockets.serve(   websocket_handler, "0.0.0.0",  8080, max_size=None):
        print("WebSocket server listening on port 8080"  )
        print( "Cloudflare Tunnel should forward to:" )
        print("http://localhost:8080" )
        await asyncio.Future()


# ---------------------------------------------------------
# START
# ---------------------------------------------------------

try:
    asyncio.run(main())

except KeyboardInterrupt:
    print("[SYSTEM] Keyboard interrupt" )

finally:
    audio_stream.stop()
    audio_stream.close()
