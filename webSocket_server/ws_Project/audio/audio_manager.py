import asyncio
import time

from audio.audio_queue import audio_queue
from audio.audio_player import AudioPlayer
from audio.audio_recorder import AudioRecorder
from audio.speech_to_text import SpeechToText


AUDIO_TIMEOUT = 1.0   # seconds without packets = end of speech


class AudioManager:

    def __init__(self):

        self.player = AudioPlayer()

        self.recorder = AudioRecorder()

        self.stt = SpeechToText()


        self.recording = False

        self.last_audio_time = 0


    async def run(self):

        print("[AUDIO MANAGER] Started")

        while True:

            try:

                # ------------------------------------
                # Wait for next audio packet
                # ------------------------------------

                packet = await asyncio.to_thread(
                    audio_queue.get
                )


                sequence_number, sample_count, sample_rate, audio_data = packet


                # ------------------------------------
                # New audio stream
                # ------------------------------------

                if not self.recording:

                    print(
                        "[AUDIO] Recording started"
                    )

                    self.recorder.start()

                    self.recording = True



                # ------------------------------------
                # Update timestamp
                # ------------------------------------

                self.last_audio_time = time.time()



                # ------------------------------------
                # PLAY AUDIO
                # ------------------------------------

                self.player.play(
                    audio_data
                )


                # ------------------------------------
                # SAVE AUDIO
                # ------------------------------------

                self.recorder.write(
                    audio_data
                )


                print(
                    f"[AUDIO] Seq:{sequence_number}"
                )


                # check if stream ended

                await self.check_timeout()



            except Exception as error:

                print(
                    "[AUDIO MANAGER ERROR]",
                    error
                )


    async def check_timeout(self):

        if not self.recording:

            return


        elapsed = (
            time.time()
            -
            self.last_audio_time
        )


        if elapsed > AUDIO_TIMEOUT:


            print(
                "[AUDIO] Recording finished"
            )


            filename = self.recorder.stop()


            self.recording = False


            if filename:


                text = self.stt.convert(
                    filename
                )


                if text:

                    print(
                        "[TEXT RESULT]",
                        text
                    )