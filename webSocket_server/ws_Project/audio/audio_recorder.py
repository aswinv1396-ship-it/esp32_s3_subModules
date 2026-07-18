import os
import wave
import time

from config.settings import *


class AudioRecorder:


    def __init__(self):

        self.file = None

        self.filename = None



    def start(self):


        os.makedirs(
            AUDIO_RECORDING_DIRECTORY,
            exist_ok=True
        )


        timestamp = time.strftime(
            "%Y%m%d_%H%M%S"
        )


        self.filename = os.path.join(

            AUDIO_RECORDING_DIRECTORY,

            f"audio_{timestamp}.wav"
        )


        self.file = wave.open(
            self.filename,
            "wb"
        )


        self.file.setnchannels(
            AUDIO_CHANNELS
        )


        self.file.setsampwidth(
            AUDIO_BITS_PER_SAMPLE // 8
        )


        self.file.setframerate(
            AUDIO_SAMPLE_RATE
        )


        print(
            "[RECORDER] Started:",
            self.filename
        )

    def write(self, audio_data):
        if self.file:
            self.file.writeframes( audio_data )

    def stop(self):

        if self.file:
            self.file.close()
            print( "[RECORDER] Saved:",   self.filename )
            filename = self.filename
            self.file = None
            self.filename = None
            return filename

        return None