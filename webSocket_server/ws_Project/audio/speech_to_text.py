import json
from vosk import Model, KaldiRecognizer

from config.settings import AUDIO_SAMPLE_RATE


class SpeechToText:


    def __init__(self):

        print("[STT] Loading model...")


        self.model = Model(
            "model/vosk-model-small-en-us-0.15"
        )


        self.recognizer = KaldiRecognizer(
            self.model,
            AUDIO_SAMPLE_RATE
        )


        print("[STT] Ready")



    def process(self, audio_data):

        """
        audio_data = raw PCM16 bytes
        """


        if self.recognizer.AcceptWaveform(
            audio_data
        ):


            result = json.loads(

                self.recognizer.Result()

            )


            text = result.get(
                "text",
                ""
            )


            if text:

                print(
                    "[FINAL TEXT]",
                    text
                )


                return text



        else:


            partial = json.loads(

                self.recognizer.PartialResult()

            )


            partial_text = partial.get(
                "partial",
                ""
            )

            if partial_text:
                print(
                    "[ENGLISH PARTIAL]",
                    partial_text
                )

                return partial_text



        return None