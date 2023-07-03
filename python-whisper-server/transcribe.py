#! python3.7

'''
Error codes:
0: successful
1: (Linux only) please select a microphone
'''

import argparse
import io
import os
import speech_recognition as sr
import whisper
import torch

from datetime import datetime, timedelta
from queue import Queue
from tempfile import NamedTemporaryFile
from time import sleep
from sys import platform

from threading import *
from socket import *
from queue import Queue
import time

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model", default="tiny", help="Model to use",
                        choices=["tiny", "base", "small", "medium", "large"])
    parser.add_argument("--non_english", action='store_true',
                        help="Don't use the english model.")
    parser.add_argument("--energy_threshold", default=1000,
                        help="Energy level for mic to detect.", type=int)
    parser.add_argument("--record_timeout", default=2,
                        help="How real time the recording is in seconds.", type=float)
    parser.add_argument("--phrase_timeout", default=3,
                        help="How much empty space between recordings before we "
                             "consider it a new line in the transcription.", type=float)  
    if 'linux' in platform:
        parser.add_argument("--default_microphone", default='pulse',
                            help="Default microphone name for SpeechRecognition. "
                                 "Run this with 'list' to view available Microphones.", type=str)
    args = parser.parse_args()
    
    # The last time a recording was retreived from the queue.
    phrase_time = None
    # Current raw audio bytes.
    last_sample = bytes()
    # Thread safe Queue for passing data from the threaded recording callback.
    data_queue = Queue()
    # We use SpeechRecognizer to record our audio because it has a nice feauture where it can detect when speech ends.
    recorder = sr.Recognizer()
    recorder.energy_threshold = args.energy_threshold
    # Definitely do this, dynamic energy compensation lowers the energy threshold dramtically to a point where the SpeechRecognizer never stops recording.
    recorder.dynamic_energy_threshold = False
    
    # Important for linux users. 
    # Prevents permanent application hang and crash by using the wrong Microphone
    if 'linux' in platform:
        mic_name = args.default_microphone
        if not mic_name or mic_name == 'list':
            print("Available microphone devices are: ")
            for index, name in enumerate(sr.Microphone.list_microphone_names()):
                print(f"Microphone with name \"{name}\" found")  
            print('Please select a microphone and rerun the program')
            sys.exit(1)
            return
        else:
            for index, name in enumerate(sr.Microphone.list_microphone_names()):
                if mic_name in name:
                    source = sr.Microphone(sample_rate=16000, device_index=index)
                    break
    else:
        source = sr.Microphone(sample_rate=16000)
        
    # Load / Download model
    model = args.model
    if args.model != "large" and not args.non_english:
        model = model + ".en"
    audio_model = whisper.load_model(model)

    record_timeout = args.record_timeout
    phrase_timeout = args.phrase_timeout

    temp_file = NamedTemporaryFile().name
    transcription = ['']

    print('temp file:', temp_file)
    
    with source:
        recorder.adjust_for_ambient_noise(source)

    def record_callback(_, audio:sr.AudioData) -> None:
        """
        Threaded callback function to recieve audio data when recordings finish.
        audio: An AudioData containing the recorded bytes.
        """
        # Grab the raw bytes and push it into the thread safe queue.
        data = audio.get_raw_data()
        data_queue.put(data)

    # Create a background thread that will pass us raw audio bytes.
    # We could do this manually but SpeechRecognizer provides a nice helper.
    recorder.listen_in_background(source, record_callback, phrase_time_limit=record_timeout)

    # Cue the user that we're ready to go.
    print("Model loaded.\n")

    global server
    global running
    global messages
    server = Server()

    phrase_time = None

    index = 0

    t2 = Thread(target=server.listen_conn)
    t2.start()

    while running:
        try:
            now = datetime.utcnow()
            # Pull raw recorded audio from the queue.
            if not data_queue.empty():
                phrase_complete = False
                # If enough time has passed between recordings, consider the phrase complete.
                # Clear the current working audio buffer to start over with the new data.
                if phrase_time and now - phrase_time > timedelta(seconds=phrase_timeout):
                    last_sample = bytes()
                    phrase_complete = True
                # This is the last time we received new audio data from the queue.
                phrase_time = now

                # Concatenate our current audio data with the latest audio data.
                while not data_queue.empty():
                    data = data_queue.get()
                    last_sample += data

                # Use AudioData to convert the raw data to wav data.
                audio_data = sr.AudioData(last_sample, source.SAMPLE_RATE, source.SAMPLE_WIDTH)
                wav_data = io.BytesIO(audio_data.get_wav_data())

                # Write wav data to the temporary file as bytes.
                with open(temp_file, 'w+b') as f:
                    f.write(wav_data.read())

                # Read the transcription.
                t0 = time.time()
                
                result = audio_model.transcribe(temp_file, fp16=torch.cuda.is_available())
                text = result['text'].strip()

                t1 = time.time()

                total = t1-t0

                # If we detected a pause between recordings, add a new item to our transcripion.
                # Otherwise edit the existing one.
                if phrase_complete:
                    index += 1

                # print the current line, with an id at the beginning so we can tell if it's
                # a new section or an update on an existing one
                print(index, text, flush=True)
                server.add_message(index, text + '\n')

                # Infinite loops are bad for processors, must sleep.
                sleep(0.25)
        except KeyboardInterrupt as e:
            print(e)
            running = False
            close_server()
            server.close_conns()
            break

server = None

class Client():
    def __init__(self, sock, addr):
        self.sock = sock 
        self.addr = addr 
        self.thread = None
        self.messages = Queue()
        pass

    def send_msg(self, idx, text):
        self.messages.put((idx, text))

    def run(self):
        global running

        while running:
            idx, text = self.messages.get()

            if idx == -1 and text == 'stop':
                break

            self.sock.send(bytes(str(idx) + text, 'UTF-8'))

running = True
num_clients = 0

class Server():
    def __init__(self):
        self.clients = []

    def close_conns(self):
        for c in self.clients:
            c.send_msg(-1, 'stop')

    def listen_conn(self):
        global num_clients

        TCP_IP = '127.0.0.1'
        TCP_PORT = 5005

        with socket(AF_INET, SOCK_STREAM) as s:
            s.bind((TCP_IP, TCP_PORT))
            s.listen()

            while running:
                client_sock, addr = s.accept()

                if not running:
                    client_sock.close()
                    break

                c = Client(client_sock, addr)
                new_thread = Thread(target=c.run)
                new_thread.daemon = True
                c.thread = new_thread
                new_thread.start()
                
                num_clients += 1

                self.clients.append(c)

    def add_message(self, idx, text):
        for c in self.clients:
            c.send_msg(idx, text)

def close_server():
    # Create a socket
    sock = socket(AF_INET, SOCK_STREAM)

    # Connect to the remote host and port
    sock.connect(('127.0.0.1', 5005))

    # Send a request to the host
    sock.send(bytes("stop\r\n", 'UTF-8'))

    # Terminate
    sock.close()

if __name__ == "__main__":
    main()