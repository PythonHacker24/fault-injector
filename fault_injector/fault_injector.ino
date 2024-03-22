/* 
  Fault Injector for ESP8266 (nodeMCU) and Arduino (ESP8266 is preferred due to it's clock speed)

  Author: Aditya Patil <Hack the Planet!>

  Fault Injection Attack can be used for attacking electronic circuits in various ways.
  Requirements:
    Push Button - Trigger Button 
    Jumper Wires 
    Pull Down Resistors - 10K Ohms for Fault Pin 

  Parameters can be varied in the code for adjusting pinouts. 
  Important Prameters: 
    TRIGGER_PIN - Pin for Push Button Input (Fault Trigger)
    FAULT_PIN - Pin for Fault Injection (Always HIGH and glitches when Trigger is pushed)
    faultDuration - Duration for Fault (Time interval to become LOW)
    maxFaultDuration - Maximum Fault Duration (Maximum Fault Injection Interval)
    triggerDebounceDelay - Debounce Delay for Trigger (Optional, default works just fine)
*/

// Notes: For Analog Fault Injections, use PWM Signals and a Low Pass Filter
// Add Setp by Step Incremental and Decremental Functions, but first test the current functionalities
  // Initial Duration and Increment Factor or Decrement Factor must be the arguements 
// Add 2 modes of usage 
  // 1. IC2 Display mode: A portable fault injection device
  // 2. Serial Communication Control: Fault Injection Attacks with Serial Console
  // 3. Add Clock Fault Injection Support: When Pulse is HIGH, glitch with LOW Voltage. When Pulse is LOW, glitch with HIGH Voltage.

#include <ESP8266WiFi.h>

#define TRIGGER_PIN D1                // Trigger Input Pin (Pull Down Register is Required)
#define DIGITAL_FAULT_PIN D2          // Digital Output Pin
#define ANALOG_FAULT_PIN D3           // Analog Output Pin
#define INTERRUPT_PIN D4              // Interupt Input Pin (Pull Down Register is Required)

// Default Define Normal State and Fault State 
byte normalState = 0x0;
byte faultState = 0x1; 

// Parameters Fault Duration
int faultDuration = 0;
int maxFaultDuration = 0; 

// Trigger Debounce Mechanism Declarations 
int triggerState;             // Current State of the Trigger
int lastTriggerState = LOW;   // Initialise last trigger state to LOW

// Stepping Debounce Mechanism Declarations 
int stepperState;
int lastStepperState = LOW;

unsigned long lastStepperDebounceTime = 0;
unsigned long lastTriggerDebounceTime = 0;    // Last time when the Trigger was pressed 
unsigned long universalDebounceDelay = 10;      // Debounce Delay time for all buttons 

int counter = 0;   

// Declare States of Operation in Fault Pin
void state_declaration(int state) {
  if (state == 0) {
    normalState = 0x0;
    faultState = 0x1;
  } else if (state == 1) {
    normalState = 0x1;
    faultState = 0x0;
  } else {
    exit(1);
  }
}

void setup() {
  pinMode(TRIGGER_PIN, INPUT);                  // Trigger Button will be connected here
  pinMode(INTERRUPT_PIN, INPUT);                // Interrupt Signal 
  pinMode(DIGITAL_FAULT_PIN, OUTPUT);           // Digital Fault Injection Pin 
  pinMode(ANALOG_FAULT_PIN, OUTPUT);            // Analog Fault Injection Pin

  analogWriteFreq(1000);                        // Setting PWM Frequency to 1000 Hz 
  analogWriteResolution(10);                    // Setting PWM Resolution to 10 bits (1024 Levels)

  maxFaultDuration = 100;
  state_declaration(1);

  digitalWrite(DIGITAL_FAULT_PIN, normalState);        // Fault Pin State to HIGH
  Serial.begin(115200); 

  Serial.setDebugOutput(false);     // Disable debug output
  Serial.flush();                   // Clear any remaining data in the serial buffer
}

// Digital Fault Injection Function - Glitch Power Suppy from 1 to 0 (total supply cut)
void digital_fault_injector(int fault_duration) {
  // Glitch Portion: Set Fault Pin to Low and then up in Fault Duration
  digitalWrite(DIGITAL_FAULT_PIN, faultState);
  delayMicroseconds(fault_duration);
  digitalWrite(DIGITAL_FAULT_PIN, normalState); // Set back to normal state
}

// Digital Fault Injection Function with Interrupt
void digital_interrupted_fault_injector(int fault_duration) {
  int interruptReading = digitalRead(INTERRUPT_PIN); 
  if (interruptReading == HIGH) {
    digital_fault_injector(fault_duration);
    Serial.println("Fault Injected on Interrupt!");
  }
}

// Notes: Combine Analog and Digital Variable Incrmental and Decremental Functions 

// Variable Incremental Fault Injector Function
void digital_incremental_interrupt_fault_injector(int initialDuration, int maxFaultDuration, int incrementFactor) {
  // D4 will be the interrupt reading
  int interruptReading;
  for (int downTime = 0; downTime < maxFaultDuration; downTime = downTime + incrementFactor) {
    interruptReading = digitalRead(INTERRUPT_PIN);
    if (interruptReading == HIGH) { 
      break; 
    }
    digital_fault_injector(downTime);
    Serial.print("Downtime: ");
    Serial.println(downTime);
    delay(500);
  }
}

// Variable Decremental Fault Injector Funcrion
void digital_decremental_interrupt_fault_injector(int initialDuration, int maxFaultDuration, int decrementFactor) {
  // D4 will be the interrupt reading
  int interruptReading;
  for (int downTime = maxFaultDuration; downTime > initialDuration; downTime = downTime - decrementFactor) {
    interruptReading = digitalRead(INTERRUPT_PIN);
    if (interruptReading == HIGH) {
      break;
    }
    digital_fault_injector(downTime);
    Serial.print("Downtime: ");
    Serial.println(downTime);
    delay(500);
  }
}

// Stepping up and down allows user to step up or down the voltage by some factor with buttons. 
// This is particularly helpful for debugging manually and finding the correct time duration for fault injection
void step_up_digital_incremental_interrupt_fault_injector(int initialDuration, int incrementFactor) {
  int intervalFactor = 0;
  while (true) {
    // Listen for the stepping triggers 
    while (true) {
      
      int stepperReading = digitalRead(INTERRUPT_PIN);
      if (stepperReading != lastStepperState) {
        lastStepperDebounceTime = millis();
      }
      if ((millis() - lastStepperDebounceTime) > universalDebounceDelay) {
        if (stepperReading != stepperState) {
          stepperState = stepperReading;

          if (lastStepperState == LOW) {
            digital_fault_injector(initialDuration + intervalFactor);
          }
          break;
        }
      }
      lastStepperState = stepperReading;
    }
    intervalFactor += incrementFactor;
  }
}

void step_down_digital_decremental_interrupt_fault_injector(int initialDuration, int decrementFactor) {
  int intervalFactor = 0;
  while (true) {
    // Listen for the stepping triggers 
    while (true) {
      
      int stepperReading = digitalRead(INTERRUPT_PIN);
      if (stepperReading != lastStepperState) {
        lastStepperDebounceTime = millis();
      }
      if ((millis() - lastStepperDebounceTime) > universalDebounceDelay) {
        if (stepperReading != stepperState) {
          stepperState = stepperReading;

          if (lastStepperState == LOW) {
            digital_fault_injector(initialDuration - intervalFactor);
          }
          break;
        }
      }
      lastStepperState = stepperReading;
    }
    intervalFactor += decrementFactor;
  }
}

void loop() {

  // Listen for the Trigger
  while (true) {

    // Debounce Mechanism
    int triggerReading = digitalRead(TRIGGER_PIN);  // Read the Trigger Pin 
    if (triggerReading != lastTriggerState) {
      lastTriggerDebounceTime = millis();           // Reset the Trigger Debounce Timer 
    }

    if ((millis() - lastTriggerDebounceTime) > universalDebounceDelay) {    // Check if Debounce Delay for Trigger Pin has been passed
      if (triggerReading != triggerState) {
        triggerState = triggerReading;         // Set Trigger State to new Trigger Reading 

        // Fault Injection Function
        if (lastTriggerState == LOW) {        // Inject Fault if State changes from LOW to HIGH
          digital_incremental_interrupt_fault_injector(0, maxFaultDuration, 1);

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