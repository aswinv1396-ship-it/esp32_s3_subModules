import asyncio

from audio.audio_queue import (
    player_queue,
    record_queue,
    stt_queue
)

from audio.audio_player import AudioPlayer
from audio.audio_recorder import AudioRecorder
from audio.speech_to_text import SpeechToText


class AudioManager:


    def __init__(self):

        print("[AUDIO MANAGER] Initializing")


        self.player = AudioPlayer()

        self.recorder = AudioRecorder()

        self.stt = SpeechToText()

    async def run(self):

        print(
            "[AUDIO MANAGER] Running"
        )


        await asyncio.gather(

            self.player_task(),

            self.recorder_task(),

            self.stt_task()

        )

    async def start(self):

        print("[AUDIO MANAGER] Started")


        await asyncio.gather(

            self.player_task(),

            self.recorder_task(),

            self.stt_task()

        )



    # ======================================================
    # AUDIO PLAYBACK TASK
    # ======================================================

    async def player_task(self):

        print(
            "[AUDIO PLAYER TASK] Running"
        )


        while True:

            packet = await asyncio.to_thread(

                player_queue.get

            )


            audio_data = packet["data"]


            sequence = packet["sequence"]


            try:

                self.player.play(
                    audio_data
                )


                print(
                    f"[PLAYER] Seq:{sequence}"
                )


            except Exception as error:

                print(
                    "[PLAYER ERROR]",
                    error
                )



    # ======================================================
    # AUDIO RECORDING TASK
    # ======================================================

    async def recorder_task(self):

        print(
            "[AUDIO RECORDER TASK] Running"
        )


        recording = False



        while True:


            packet = await asyncio.to_thread(

                record_queue.get

            )


            audio_data = packet["data"]



            try:


                if not recording:


                    self.recorder.start()


                    recording = True


                    print(
                        "[RECORDER] Started"
                    )



                self.recorder.write(

                    audio_data

                )


                print(

                    f"[RECORDER] Seq:{packet['sequence']}"

                )



            except Exception as error:


                print(

                    "[RECORDER ERROR]",

                    error

                )

    async def stt_task(self):

        print(
            "[STT TASK] Running"
        )

        while True:

            packet = await asyncio.to_thread(

                stt_queue.get

            )

            audio_data = packet["data"]

            sequence = packet["sequence"]

            print(
                f"[STT] Processing Seq={sequence}"
            )

            try:

                text = self.stt.process(

                    audio_data

                )

                if text:
                    print(
                        "=============================="
                    )

                    print(
                        "[STT TEXT]"
                    )

                    print(
                        text
                    )

                    print(
                        "=============================="
                    )



            except Exception as error:

                print(
                    "[STT ERROR]",
                    error
                )