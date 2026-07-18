# ============================================================
# SYSTEM SETTINGS
# ============================================================

APPLICATION_NAME = "ESP32 Audio WebSocket Server"

APPLICATION_VERSION = "1.0.0"


# ============================================================
# NETWORK SETTINGS
# ============================================================

WEBSOCKET_HOST = "0.0.0.0"

WEBSOCKET_PORT = 8080

WEBSOCKET_MAX_SIZE = None


# ============================================================
# AUDIO SETTINGS
# ============================================================

AUDIO_SAMPLE_RATE = 16000

AUDIO_CHANNELS = 1

AUDIO_BITS_PER_SAMPLE = 16

AUDIO_BYTES_PER_SAMPLE = (
    AUDIO_BITS_PER_SAMPLE // 8
)

AUDIO_SAMPLES_PER_PACKET = 512


# ============================================================
# AUDIO PACKET TIMING
# ============================================================

AUDIO_PACKET_DURATION_MS = (

    AUDIO_SAMPLES_PER_PACKET
    * 1000
    / AUDIO_SAMPLE_RATE

)


# ============================================================
# AUDIO BUFFER SETTINGS
# ============================================================

AUDIO_BUFFER_MAX_PACKETS = 20

AUDIO_BUFFER_INITIAL_PACKETS = 10

AUDIO_BUFFER_MIN_PACKETS = 5

AUDIO_BUFFER_WARNING_PACKETS = 3


# ============================================================
# AUDIO BUFFER TIMING
# ============================================================

AUDIO_BUFFER_INITIAL_MS = (

    AUDIO_BUFFER_INITIAL_PACKETS
    * AUDIO_PACKET_DURATION_MS

)


AUDIO_BUFFER_MINIMUM_MS = (

    AUDIO_BUFFER_MIN_PACKETS
    * AUDIO_PACKET_DURATION_MS

)


AUDIO_BUFFER_WARNING_MS = (

    AUDIO_BUFFER_WARNING_PACKETS
    * AUDIO_PACKET_DURATION_MS

)


# ============================================================
# AUDIO PLAYBACK SETTINGS
# ============================================================

AUDIO_PLAYBACK_BLOCKSIZE = (

    AUDIO_SAMPLES_PER_PACKET

)


AUDIO_PLAYBACK_DTYPE = "int16"


# ============================================================
# AUDIO STORAGE SETTINGS
# ============================================================

AUDIO_RECORDING_DIRECTORY = (

    "data/recordings"

)


AUDIO_RECORDING_FILE_PREFIX = (

    "audio_recording"

)


AUDIO_RECORDING_FORMAT = "WAV"


# ============================================================
# AUDIO PROCESSING SETTINGS
# ============================================================

AUDIO_NOISE_FILTER_ENABLED = False

AUDIO_GAIN_ENABLED = False

AUDIO_RESAMPLING_ENABLED = False


# ============================================================
# SPEECH-TO-TEXT SETTINGS
# ============================================================

SPEECH_TO_TEXT_ENABLED = False

SPEECH_LANGUAGE = "en"


# ============================================================
# AI AGENT SETTINGS
# ============================================================

AI_AGENT_ENABLED = False


# ============================================================
# ESP32 RESPONSE SETTINGS
# ============================================================

ESP32_RESPONSE_ENABLED = False


# ============================================================
# DEBUG SETTINGS
# ============================================================

DEBUG_ENABLED = True

PRINT_AUDIO_PACKET_INFO = False

PRINT_NETWORK_INFO = True

PRINT_BUFFER_INFO = True


# ============================================================
# STATISTICS SETTINGS
# ============================================================

STATISTICS_PRINT_INTERVAL_SECONDS = 5

# ============================================================
# AUDIO PACKET PROTOCOL
# ============================================================

import struct


AUDIO_PACKET_MAGIC = 0x41554430

AUDIO_PACKET_VERSION = 1

AUDIO_PACKET_HEADER_FORMAT = "<IBBHII"

AUDIO_PACKET_HEADER_SIZE = struct.calcsize(AUDIO_PACKET_HEADER_FORMAT)