from queue import Queue

# Audio playback queue
player_queue = Queue(maxsize=200)
# WAV recording queue
record_queue = Queue(maxsize=200)
# Speech-to-text queue
stt_queue = Queue(maxsize=200)