import speech_recognition as sr


class SpeechToText:


    def __init__(self):

        self.recognizer = sr.Recognizer()



    def convert(self, filename):

        print(
            "[STT] Processing:",
            filename
        )


        try:

            with sr.AudioFile(
                filename
            ) as source:


                audio = self.recognizer.record(
                    source
                )


            text = self.recognizer.recognize_google(
                audio
            )


            print(
                "[STT]",
                text
            )


            return text



        except sr.UnknownValueError:


            print(
                "[STT] Could not understand audio"
            )


        except Exception as error:


            print(
                "[STT ERROR]",
                error
            )


        return None