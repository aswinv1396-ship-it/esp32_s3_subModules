import time

from audio.audio_queue import (
    player_queue,
    record_queue,
    stt_queue
)


class AudioParser:


    def __init__(self):

        print("[AUDIO PARSER] Initialized")



    async def parse(
        self,
        audio_data,
        sequence_number,
        sample_count,
        sample_rate
    ):


        packet = {

            "sequence": sequence_number,

            "timestamp": time.time(),

            "sample_count": sample_count,

            "sample_rate": sample_rate,

            "data": audio_data

        }



        # -----------------------------------------
        # SEND TO PLAYER
        # -----------------------------------------

        try:

            player_queue.put_nowait(
                packet
            )

        except:

            print(
                "[PLAYER QUEUE] Overflow"
            )



        # -----------------------------------------
        # SEND TO RECORDER
        # -----------------------------------------

        try:

            record_queue.put_nowait(
                packet
            )

        except:

            print(
                "[RECORD QUEUE] Overflow"
            )



        # -----------------------------------------
        # SEND TO STT
        # -----------------------------------------

        try:
            stt_queue.put_nowait(packet )
            print(f"[STT QUEUE] Added Seq={sequence_number}"  )
        except:

            print(
                "[STT QUEUE] Overflow"
            )



        # -----------------------------------------
        # DEBUG
        # -----------------------------------------

        print(
            f"[AUDIO PARSER] "
            f"Seq={sequence_number} "
            f"Samples={sample_count} "
            f"Rate={sample_rate}"
        )