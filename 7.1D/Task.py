import speech_recognition as sr
import RPi.GPIO as GPIO
import time

# GPIO setup
LED_PIN = 17
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.output(LED_PIN, GPIO.LOW)  # LED initially off

# Initialize the recognizer
recognizer = sr.Recognizer()

def listen_for_command():
    with sr.Microphone() as source:
        print("Please say a command (turn on or turn off the light):")
        recognizer.adjust_for_ambient_noise(source, duration=1)
        audio = recognizer.listen(source)
        try:
            command = recognizer.recognize_google(audio)
            print("You said:", command)
            return command.lower()
        except sr.UnknownValueError:
            print("Sorry, I could not understand the audio.")
            return None
        except sr.RequestError as e:
            print(f"Could not request results; {e}")
            return None

def control_led(command):
    if command:
        if 'turn on' in command:
            GPIO.output(LED_PIN, GPIO.HIGH)
            print("LED turned ON")
        elif 'turn off' in command:
            GPIO.output(LED_PIN, GPIO.LOW)
            print("LED turned OFF")
        else:
            print("Command not recognized. Please say 'turn on' or 'turn off'.")
    else:
        print("No valid command received.")

try:
    while True:
        command = listen_for_command()
        control_led(command)
        time.sleep(1)  # Optional delay before next command
except KeyboardInterrupt:
    print("Program stopped by user.")
finally:
    GPIO.cleanup()
