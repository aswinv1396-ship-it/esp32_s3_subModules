import struct

from config.settings import (
    AUDIO_PACKET_MAGIC,
    AUDIO_PACKET_VERSION,
    AUDIO_CHANNELS,
    AUDIO_SAMPLE_RATE,
    AUDIO_SAMPLES_PER_PACKET,
    AUDIO_PACKET_HEADER_FORMAT,
    AUDIO_PACKET_HEADER_SIZE
)

from audio.audio_parser import AudioParser


class DataProcessing:

    def __init__(self, audio_manager):

        self.audio_parser = AudioParser()

    async def process(self, message):

        # ---------------------------------------------
        # TEXT MESSAGE
        # ---------------------------------------------

        if isinstance(message, str):

            print(f"[TEXT] {message}")

            return

        if not isinstance(message, bytes):
            print("[ERROR] Unknown Message Type")
            return


        if len(message) < AUDIO_PACKET_HEADER_SIZE:
            print("[ERROR] Packet Too Small")
            return

        # ---------------------------------------------
        # UNPACK HEADER
        # ---------------------------------------------

        try:
            (
                magic,
                version,
                channels,
                sample_count,
                sample_rate,
                sequence_number
            ) = struct.unpack(
                AUDIO_PACKET_HEADER_FORMAT,
                message[:AUDIO_PACKET_HEADER_SIZE]
            )

        except Exception as error:
            print(f"[ERROR] Header Parse Failed : {error}")
            return

        # ---------------------------------------------
        # VALIDATE MAGIC
        # ---------------------------------------------

        if magic != AUDIO_PACKET_MAGIC:
            print(f"[ERROR] Invalid Magic : {hex(magic)}")
            return

        # ---------------------------------------------
        # VALIDATE VERSION
        # ---------------------------------------------

        if version != AUDIO_PACKET_VERSION:
            print(f"[ERROR] Unsupported Version : {version}")
            return

        # ---------------------------------------------
        # VALIDATE CHANNELS
        # ---------------------------------------------

        if channels != AUDIO_CHANNELS:
            print(f"[ERROR] Invalid Channels : {channels}")
            return

        # ---------------------------------------------
        # VALIDATE SAMPLE RATE
        # ---------------------------------------------

        if sample_rate != AUDIO_SAMPLE_RATE:
            print(f"[ERROR] Invalid Sample Rate : {sample_rate}")
            return

        # ---------------------------------------------
        # VALIDATE SAMPLE COUNT
        # ---------------------------------------------

        if sample_count > AUDIO_SAMPLES_PER_PACKET:
            print(f"[ERROR] Invalid Sample Count : {sample_count}")
            return

        # ---------------------------------------------
        # VALIDATE AUDIO SIZE
        # ---------------------------------------------

        audio_data = message[AUDIO_PACKET_HEADER_SIZE:]

        expected_size = sample_count * 2

        if len(audio_data) != expected_size:
            print("[ERROR] Invalid Audio Size")
            print(f"Expected : {expected_size}")
            print(f"Received : {len(audio_data)}")
            return

        print(  f"[AUDIO] Seq={sequence_number} "   f"Samples={sample_count}"  )

        await self.audio_parser.parse( audio_data, sequence_number, sample_count,  sample_rate  )