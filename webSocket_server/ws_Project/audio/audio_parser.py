from audio.audio_queue import audio_queue


class AudioParser:

    def __init__(self, audio_manager):
        self.audio_manager = audio_manager

    async def parse(self,  audio_data, sequence_number, sample_count, sample_rate  ):

        print(
            f"[AUDIO PARSER] "
            f"Seq={sequence_number} "
            f"Samples={sample_count} "
            f"Rate={sample_rate}"
        )
        audio_queue.put(
            (
                sequence_number,
                sample_count,
                sample_rate,
                audio_data
            )
        )