import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)
GPIO.setup(7, GPIO.OUT)

try:
    while True:  
        GPIO.output(7, GPIO.HIGH)   
        time.sleep(0.25)
        GPIO.output(7, GPIO.LOW)   
        time.sleep(0.25)

except KeyboardInterrupt:
    GPIO.cleanup()
