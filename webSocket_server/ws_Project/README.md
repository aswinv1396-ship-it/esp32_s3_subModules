# ESP32 WebSocket Audio Server

## 1. Project Overview

This project provides a modular Python WebSocket server for communication between an ESP32-S3 device and a Linux host system.

The architecture is designed to receive:

* Binary audio packets from ESP32
* Text messages from ESP32
* Audio data for playback
* Audio data for storage
* Audio data for future speech-to-text processing
* Audio data for future AI-agent processing

The system is designed in a modular way so that new functionality can be added without modifying the core WebSocket communication layer.

---

## 2. Project Structure

```text
ws_Project/
│
├── README.md
├── requirements.txt
├── main.py
│
├── config/
│   ├── __init__.py
│   └── settings.py
│
├── network/
│   ├── __init__.py
│   └── websocket_server.py
│
├── audio/
│   ├── __init__.py
│   ├── audio_packet.py
│   ├── audio_playback.py
│   ├── audio_storage.py
│   └── audio_buffer.py
│
├── speech/
│   ├── __init__.py
│   └── speech_to_text.py
│
└── ai/
    ├── __init__.py
    └── ai_agent.py
```

---

## 3. System Architecture

```text
                    ┌──────────────────┐
                    │      ESP32-S3    │
                    │                  │
                    │  Microphone      │
                    │      │           │
                    │      ▼           │
                    │  Audio Capture   │
                    │      │           │
                    │      ▼           │
                    │  Audio Packet    │
                    │      │           │
                    └──────┼───────────┘
                           │
                           │ WebSocket
                           │
                           ▼
                 ┌────────────────────┐
                 │  Python WebSocket  │
                 │      Server         │
                 └─────────┬──────────┘
                           │
                           ▼
                       main.py
                           │
             ┌─────────────┼─────────────┐
             │             │             │
             ▼             ▼             ▼
       Audio Playback  Audio Storage  Speech-to-Text
                                           │
                                           ▼
                                      AI Agent
                                           │
                                           ▼
                                  Response to ESP32
```

---

## 4. Python Version

Recommended:

```text
Python 3.10 or newer
```

Check the installed Python version:

```bash
python3 --version
```

---

## 5. Virtual Environment Setup

Go to the project directory:

```bash
cd /home/aswin/esp/projects/esp32_s3_subModules/webSocket_server/ws_Project
```

Create a virtual environment:

```bash
python3 -m venv .venv
```

Activate the virtual environment:

```bash
source .venv/bin/activate
```

After activation, the terminal should show:

```text
(.venv)
```

---

## 6. Install Python Dependencies

Install all Python dependencies:

```bash
python -m pip install --upgrade pip
```

Then:

```bash
python -m pip install -r requirements.txt
```

---

## 7. System Dependency for Audio

The Python `sounddevice` module requires the PortAudio library.

Install the required Linux packages:

```bash
sudo apt update
```

```bash
sudo apt install libportaudio2 portaudio19-dev
```

After installation, verify the audio module:

```bash
python -c "import sounddevice; print('sounddevice OK')"
```

---

## 8. Verify Python Dependencies

Run:

```bash
python -c "import websockets; print('websockets OK')"
```

Run:

```bash
python -c "import numpy; print('numpy OK')"
```

Run:

```bash
python -c "import sounddevice; print('sounddevice OK')"
```

Or verify all modules together:

```bash
python -c "import websockets, numpy, sounddevice; print('All dependencies OK')"
```

Expected output:

```text
All dependencies OK
```

---

## 9. WebSocket Server

The WebSocket communication is implemented in:

```text
network/websocket_server.py
```

The server is responsible for:

* Opening the WebSocket server
* Accepting ESP32 connections
* Receiving messages
* Detecting binary and text messages
* Forwarding received messages to the application callback
* Handling client disconnection

The WebSocket server does not directly process audio.

This separation allows audio processing to be modified independently.

---

## 10. Application Entry Point

The main application is:

```text
main.py
```

Run the server from the project root:

```bash
python main.py
```

The server listens on:

```text
0.0.0.0:8080
```

The WebSocket endpoint is:

```text
ws://<SERVER_IP>:8080
```

---

## 11. Current Message Flow

### Binary Message

```text
ESP32
  │
  ▼
WebSocket Server
  │
  ▼
process_received_message()
  │
  ▼
Binary Audio Data
```

### Text Message

```text
ESP32
  │
  ▼
WebSocket Server
  │
  ▼
process_received_message()
  │
  ▼
Text Processing
```

---

## 12. Audio Packet Format

The audio packet uses the following format:

```text
+-----------------------+
| Magic                 |
+-----------------------+
| Version               |
+-----------------------+
| Channels              |
+-----------------------+
| Sample Count          |
+-----------------------+
| Sample Rate           |
+-----------------------+
| Sequence Number       |
+-----------------------+
| PCM Audio Samples     |
+-----------------------+
```

Current audio configuration:

```text
Sample Rate        : 16000 Hz
Channels           : 1
Sample Format      : Signed 16-bit PCM
Samples per Packet : 512
Packet Duration    : 32 ms
```

The packet duration is calculated as:

```text
512 samples / 16000 samples per second
= 0.032 seconds
= 32 ms
```

---

## 13. Audio Data Rate

For the current configuration:

```text
16000 samples/second
× 2 bytes/sample
× 1 channel
= 32000 bytes/second
```

Therefore, the raw audio data rate is:

```text
32 KB/s
```

The audio packet payload is:

```text
512 samples × 2 bytes
= 1024 bytes
```

The audio packet is transmitted approximately:

```text
16000 / 512
≈ 31.25 packets/second
```

---

## 14. Future Audio Processing

The audio processing will be divided into separate modules:

```text
audio/
│
├── audio_packet.py
│       │
│       └── Decode and validate packets
│
├── audio_buffer.py
│       │
│       └── Store packets temporarily
│
├── audio_playback.py
│       │
│       └── Play PCM audio
│
└── audio_storage.py
        │
        └── Save audio for future use
```

The objective is to avoid putting all functionality inside:

```text
main.py
```

---

## 15. Future Speech-to-Text Processing

The speech processing module will receive audio from the audio pipeline:

```text
ESP32
  │
  ▼
WebSocket
  │
  ▼
Audio Packet Decoder
  │
  ▼
Audio Buffer
  │
  ▼
Speech-to-Text
  │
  ▼
Text
```

The speech module will be implemented separately:

```text
speech/speech_to_text.py
```

---

## 16. Future AI Agent Integration

The planned future architecture is:

```text
ESP32 Audio
      │
      ▼
Python WebSocket Server
      │
      ▼
Audio Processing
      │
      ▼
Speech-to-Text
      │
      ▼
Text
      │
      ▼
AI Agent
      │
      ▼
AI Response
      │
      ▼
WebSocket
      │
      ▼
ESP32
```

This allows the ESP32 to send an audio command and receive a response from the AI system.

---

## 17. Running the Project

Activate the virtual environment:

```bash
source .venv/bin/activate
```

Start the server:

```bash
python main.py
```

Expected output:

```text
[WEBSOCKET] Starting server on 0.0.0.0 8080
[WEBSOCKET] Server listening on ws://0.0.0.0:8080
```

When the ESP32 connects:

```text
[WEBSOCKET] Client connected
```

When binary audio is received:

```text
[MAIN] Binary data received
```

When text is received:

```text
[MAIN] Text received: ESP32_TEST_PACKET
```

---

## 18. Troubleshooting

### Error: `No module named websockets`

Activate the virtual environment:

```bash
source .venv/bin/activate
```

Install the dependencies:

```bash
python -m pip install -r requirements.txt
```

---

### Error: `PortAudio library not found`

Install the Linux dependency:

```bash
sudo apt install libportaudio2 portaudio19-dev
```

Then reinstall the Python package if required:

```bash
python -m pip install sounddevice
```

---

### Error: WebSocket connection refused

Verify that the server is running:

```bash
python main.py
```

Verify that port `8080` is listening:

```bash
ss -tlnp | grep 8080
```

The expected result should show port:

```text
8080
```

---

## 19. Development Principle

Each subsystem should have a single responsibility.

```text
network/
    WebSocket communication

audio/
    Audio processing

speech/
    Speech-to-text processing

ai/
    AI processing

config/
    Configuration

main.py
    Application coordination
```

The WebSocket server should not contain:

* Audio playback logic
* Speech-to-text logic
* AI logic
* File-storage logic

This modular structure makes it easier to test, modify, and extend the system in the future.

