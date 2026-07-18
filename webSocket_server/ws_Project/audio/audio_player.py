import sounddevice as sd

from config.settings import *


class AudioPlayer:

    def __init__(self):

        self.stream = sd.RawOutputStream(
            samplerate=AUDIO_SAMPLE_RATE,
            channels=AUDIO_CHANNELS,
            dtype="int16",
            blocksize=AUDIO_SAMPLES_PER_PACKET
        )

        self.stream.start()

    def play(self, audio):

        self.stream.write(audio)

    def close(self):

        self.stream.stop()

        self.stream.close()