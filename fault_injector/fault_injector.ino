/* 
  Fault Injector for ESP8266 (nodeMCU) and Arduino (ESP8266 is preferred due to it's clock speed)

  Author: Aditya Patil <Hack the Planet!>

  Fault Injection Attack can be used for attacking electronic circuits in various ways.
  Requirements:
    Push Button - Trigger Button 
    Jumper Wires 
    Pull Down Resisitors - 10K Ohms for Fault Pin 

  Parameters can be varied in the code for adjusting pinouts. 
  Important Prameters: 
    TRIGGER_PIN - Pin for Push Button Input (Fault Trigger)
    FAULT_PIN - Pin for Fault Injection (Always HIGH and glitches when Trigger is pushed)
    FAULT_DURATION - Duration for Fault (Time interval to become LOW)
    triggerDebounceDelay - Debounce Delay for Trigger (Optional, default works just fine)
*/

#define TRIGGER_PIN D1        // Input Pin 
#define FAULT_PIN D8          // Output Pin

// Fault Duration
#define FAULT_DURATION 50

// Trigger Debounce Mechanism Declarations 
int triggerState;             // Current State of the Trigger
int lastTriggerState = LOW;   // Initialise last trigger state to LOW

unsigned long lastTriggerDebounceTime = 0;    // Last time when the Trigger was pressed 
unsigned long triggerDebounceDelay = 10;      // Debounce Delay time for Trigger

int counter = 0;   

void setup() {
  pinMode(TRIGGER_PIN, INPUT);          // Button will be connected here 
  // pinMode(RESET_PIN, INPUT);            // Reset Button will be connected here 
  pinMode(FAULT_PIN, OUTPUT);           // Fault Injection Pin 
  // pinMode(READY_INDICATOR, OUTPUT);     // Ready State Indicator

  digitalWrite(FAULT_PIN, HIGH);        // Fault Pin State to HIGH
  // digitalWrite(READY_INDICATOR, HIGH);  // Ready Indicator State to HIGH   
  Serial.begin(115200); 
}

// Digital Fault Injection Function - Glitch Power Suppy from 1 to 0 (total supply cut)
void digital_fault_injector(int fault_duration) {
  // Glitch Portion: Set Fault Pin to Low and then up in Fault Duration
  digitalWrite(FAULT_PIN, LOW);
  delayMicroseconds(fault_duration);
  delay(fault_duration);
  digitalWrite(FAULT_PIN, HIGH);
}

void loop() {

  // Listen for the Trigger
  while (true) {

    // Debounce Mechansim 
    int triggerReading = digitalRead(TRIGGER_PIN);  // Read the Trigger Pin 
    if (triggerReading != lastTriggerState) {
      lastTriggerDebounceTime = millis();           // Reset the Trigger Debounce Timer 
    }

    if ((millis() - lastTriggerDebounceTime) > triggerDebounceDelay) {    // Check if Debounce Delay for Trigger Pin has been passed
      if (triggerReading != triggerState) {
        triggerState = triggerReading;         // Set Trigger State to new Trigger Reading 

        // Fault Injection Function
        if (lastTriggerState == LOW) {        // Inject Fault if State changes from LOW to HIGH
          digital_fault_injector(FAULT_DURATION);
          counter++;
          Serial.print("Fault Number: ");
          Serial.println(counter);
        }
        break;                                // Break out of the loop
      }
    }

    lastTriggerState = triggerReading;    // Update the last trigger state 
  }
}
