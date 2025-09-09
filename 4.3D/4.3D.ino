#include <SAMDTimerInterrupt.h>
#include <DHT.h> 

// Pin Definitions 
#define BUTTON_PIN 2      
#define DHT_PIN    11     // DHT22 data pin
#define TRIG_PIN   9     
#define ECHO_PIN   10    
#define LED1_PIN   5    
#define LED2_PIN   6    // Now controlled by DHT22 temperature
#define LED3_PIN   7    

SAMDTimer ITimer(TIMER_TCC); 

//  DHT Sensor Setup
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// Volatile Flags for interrupt communication
volatile bool buttonPressed = false;
// volatile bool motionDetected = false; 
volatile bool timerEvent = false;

// LED States
bool led1State = LOW;
bool led2State = LOW;
bool led3State = LOW;

// Debounce timing
unsigned long lastButtonTime = 0;

// Ultrasonic distance
float lastDistance = 0.0;

void handleButton() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();

  // Software debouncing
  if (interruptTime - lastInterruptTime > 200) {
    buttonPressed = true;
  }
  lastInterruptTime = interruptTime;
}


void TimerHandler() {
  timerEvent = true;
}

// Ultrasonic sensor reading function
float readUltrasonic() {
  // Send trigger pulse
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pulse duration
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // Calculate distance in cm
  if (duration > 0) {
    lastDistance = duration * 0.0343 / 2.0;
  }

  return lastDistance;
}

// Setup function
void setup() {
  Serial.begin(115200);
  delay(2000);   // Wait for serial monitor

  Serial.println(" SIT210 Task 4.3D: Multiple Interrupts ");
  Serial.println("Using SAMDTimerInterrupt v1.7.0");
  Serial.println("Press button to toggle LED1");
  Serial.println("High temperature will toggle LED2");
  Serial.println("LED3 toggles every second via timer interrupt");
  Serial.println("Ultrasonic distance displayed every second");

  dht.begin(); //  Initialize DHT sensor

  // Initialize LED pins
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  // Initialize input pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(DHT_PIN, INPUT); // Set pinMode for DHT
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Set initial states
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(TRIG_PIN, LOW);

  // Attach external interrupts
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);

  // Initialize timer interrupt (1 second interval)
  if (ITimer.attachInterruptInterval(1000000, TimerHandler)) {
    Serial.println("Timer initialized successfully (1s interval)");
  } else {
    Serial.println("Timer initialization failed!");
  }

  Serial.println("System ready...");
}

// Main loop
void loop() {
  unsigned long currentTime = millis();

  // Handle button interrupt
  if (buttonPressed && (currentTime - lastButtonTime > 200)) {
    buttonPressed = false;
    lastButtonTime = currentTime;

    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State);
    Serial.println("Button pressed -> LED1 toggled");
  }

  // Handle timer interrupt
  if (timerEvent) {
    timerEvent = false;

    // Toggle LED3
    led3State = !led3State;
    digitalWrite(LED3_PIN, led3State);

    // Read and display ultrasonic distance
    float distance = readUltrasonic();
    Serial.print("Timer event -> LED3 toggled | Distance: ");
    Serial.print(distance);
    Serial.print(" cm");

    // Read DHT22 sensor
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    Serial.print(" | Temp: ");
    Serial.print(temp);
    Serial.print("°C, Humidity: ");
    Serial.print(hum);
    Serial.println("%");

    // Trigger LED2 based on temperature threshold
    if (temp > 25.0) {
      led2State = !led2State;
      digitalWrite(LED2_PIN, led2State);
      Serial.println("Temperature threshold crossed -> LED2 toggled");
    }
  }

  // Small delay to prevent busy waiting
  delay(10);
}