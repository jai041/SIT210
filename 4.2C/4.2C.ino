const int buttonPin = 2;     // Interrupt-capable pin for button
const int trigPin = 9;       // Trigger pin for ultrasonic sensor
const int echoPin = 10;       // Echo pin for ultrasonic sensor (interrupt-capable)
const int led1Pin = 3;       // LED controlled by button
const int led2Pin = 4;       // LED controlled by ultrasonic sensor

volatile bool buttonPressed = false;
volatile unsigned long echoStart = 0;
volatile unsigned long echoEnd = 0;
volatile int distance = 0;

bool led1State = false;
bool objectDetected = false;


const int DETECTION_THRESHOLD = 30; // Distance within which the object will be detected 

void setup() {
  Serial.begin(9600);
  while (!Serial); 
  
  pinMode(buttonPin, INPUT_PULLUP); // Enable internal pull-up resistor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  
  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(echoPin), echoISR, CHANGE);
  
  // Initial LED states
  digitalWrite(led1Pin, LOW);
  digitalWrite(led2Pin, LOW);
  
  Serial.println("System initialized. Ready for interrupts.");
}

void loop() {
  // Handle button press
  if (buttonPressed) {
    handleButtonPress();
    buttonPressed = false;
  }
  
  // Handle ultrasonic sensor detection
  handleObjectDetection();
  
  // Trigger ultrasonic sensor periodically
  static unsigned long lastTrigger = 0;
  if (millis() - lastTrigger >= 100) { // Trigger every 100ms
    triggerUltrasonic();
    lastTrigger = millis();
  }
  
  delay(10); 
}

void buttonISR() {
  buttonPressed = true;
}

void echoISR() {
  if (digitalRead(echoPin) == HIGH) {
    echoStart = micros();
  } else {
    echoEnd = micros();
    unsigned long duration = echoEnd - echoStart;
    distance = duration * 0.034 / 2; // Calculate distance in cm
  }
}

// Handler functions
void handleButtonPress() {
  led1State = !led1State;
  digitalWrite(led1Pin, led1State ? HIGH : LOW);
  
  Serial.print("Button pressed. LED1 is now ");
  Serial.println(led1State ? "ON" : "OFF");
}

void handleObjectDetection() {
  // Check if object is within detection threshold
  bool currentlyDetected = (distance > 0 && distance < DETECTION_THRESHOLD);
  
  // Only update if detection status changed
  if (currentlyDetected != objectDetected) {
    objectDetected = currentlyDetected;
    digitalWrite(led2Pin, objectDetected ? HIGH : LOW);
    
    if (objectDetected) {
      Serial.print("Object detected at ");
      Serial.print(distance);
      Serial.println("cm. LED2 turned ON.");
    } else {
      Serial.println("Object moved away. LED2 turned OFF.");
    }
  }
}

void triggerUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
}
