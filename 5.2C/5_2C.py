import tkinter as tk
import RPi.GPIO as GPIO

# --- GPIO Setup ---
GPIO.setmode(GPIO.BCM)
LED_PINS = [17, 27, 22]   # GPIO pins for LEDs
FREQ = 1000               # PWM frequency in Hz

for pin in LED_PINS:
    GPIO.setup(pin, GPIO.OUT)

# Create PWM objects
pwms = []
for pin in LED_PINS:
    pwm = GPIO.PWM(pin, FREQ)
    pwm.start(0)  # start with 0% duty cycle
    pwms.append(pwm)

# --- GUI Setup ---
root = tk.Tk()
root.title("Task 5.2C - LED Intensity Control")

# Function to update PWM duty cycle
def set_duty(index, value):
    duty = float(value)
    pwms[index].ChangeDutyCycle(duty)

# Create 3 sliders
for i, pin in enumerate(LED_PINS, start=1):
    frame = tk.Frame(root, padx=8, pady=6)
    frame.pack(fill="x")
    label = tk.Label(frame, text=f"LED {i} (GPIO {pin})")
    label.pack(anchor="w")
    slider = tk.Scale(frame, from_=0, to=100, orient=tk.HORIZONTAL,
                      length=300, command=lambda v, idx=i-1: set_duty(idx, v))
    slider.set(0)
    slider.pack()

# Cleanup when closing
def on_close():
    for pwm in pwms:
        pwm.stop()
    GPIO.cleanup()
    root.destroy()

root.protocol("WM_DELETE_WINDOW", on_close)
