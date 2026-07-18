import sounddevice as sd

from config.settings import (
    AUDIO_SAMPLE_RATE,
    AUDIO_CHANNELS,
    AUDIO_PLAYBACK_BLOCKSIZE,
    AUDIO_PLAYBACK_DTYPE
)


class AudioPlayer:


    def __init__(self):

        self.stream = sd.RawOutputStream(

            samplerate=AUDIO_SAMPLE_RATE,

            channels=AUDIO_CHANNELS,

            dtype=AUDIO_PLAYBACK_DTYPE,

            blocksize=AUDIO_PLAYBACK_BLOCKSIZE

        )


        self.stream.start()


        print(
            "[AUDIO PLAYER] Started"
        )



    def play(self, audio_data):

        self.stream.write(
            audio_data
        )



    def close(self):

        self.stream.stop()

        self.stream.close()

        print(
            "[AUDIO PLAYER] Closed"
        )