import tkinter as tk
import RPi.GPIO as GPIO

# Setup GPIO
GPIO.setmode(GPIO.BCM)
PINS = {"Red": 17, "Green": 27, "Yellow": 22}
for pin in PINS.values():
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

def select_led():
    choice = color_var.get()
    for name, pin in PINS.items():
        GPIO.output(pin, GPIO.HIGH if name == choice else GPIO.LOW)

def on_exit():
    for pin in PINS.values():
        GPIO.output(pin, GPIO.LOW)
    GPIO.cleanup()
    root.destroy()

# Tkinter GUI
root = tk.Tk()
root.title("RPi LED Controller")

color_var = tk.StringVar(value="Red")

tk.Label(root, text="Pick an LED:").pack(padx=12, pady=(12,4))

for name in ["Red", "Green", "Yellow"]:
    tk.Radiobutton(root, text=name, variable=color_var, value=name,
                   command=select_led).pack(anchor="w", padx=16)

tk.Button(root, text="Exit", command=on_exit).pack(pady=12)

# Initialize default LED state
select_led()

root.mainloop()
