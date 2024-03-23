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
  // 2. Serial Communication Control: Fault Injection Attacks with Serial Console (work on this one first)
  // 3. Add Clock Fault Injection Support: When Pulse is HIGH, glitch with LOW Voltage. When Pulse is LOW, glitch with HIGH Voltage.

#include <ESP8266WiFi.h>

uint8_t TRIGGER_PIN = D1;                // Trigger Input Pin (Pull Down Register is Required)
uint8_t DIGITAL_FAULT_PIN = D2;          // Digital Output Pin
uint8_t ANALOG_FAULT_PIN = D3;           // Analog Output Pin
uint8_t INTERRUPT_PIN = D4;              // Interupt Input Pin (Pull Down Register is Required)

// Default Define Normal State and Fault State 
byte normalState = 0x0;
byte faultState = 0x1; 

// Parameters Fault Duration
int faultDuration = 0;
int maxFaultDuration = 0; 

int incrementFactor = 0;
int decrementFactor = 0;
int initialDuration = 0; 

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
int state = 0;

// Declare States of Operation in Fault Pin
void state_declaration(int state) {
  if (state == 0) {                   // 0 state means signal is low usually and gets high during the glitch 
    normalState = 0x0;
    faultState = 0x1;
  } else if (state == 1) {            // 1 state means signal is high usually and gets low during the glitch
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
  state_declaration(state);

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
// Here, D4 is now the stepping function
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

void serial_console() {

  Serial.flush();
  const char* setCommands[] = {
    "TP",                             // Set Trigger Pin
    "DFP",                            // Set Digital Fault Pin
    "AFP",                            // Set Analog Fault Pin
    "IP",                             // Set Interrupt Pin
    "ST",                             // Set State 
    "MFD",                            // Set Maximum Fault Duration
    "UDD"                             // Set Universal Debounce Delay
    "IF",                             // Set Increment Factor 
    "DF",                             // Set Decrement Factor 
    "ID",                             // Set Initial Duration
  };

  if (Serial.available()) {                         // Check the availability of Serial Console 
    String command = Serial.readStringUntil('\n');  // Take commands from the Serial Console 

    // Parameter Setting Section 
    if (command.startsWith("set")) {
      String variableName = command.substring(4, command.indexOf(' ', 4));
      String valueString = command.substring(command.lastIndexOf(' ') + 1);
      int value = valueString.toInt();
      
      if (variableName == setCommands[0]) {
        TRIGGER_PIN = value;
        Serial.print("Trigger Pin = ");
        Serial.println(value);
      } else if (variableName == setCommands[1]) {
        DIGITAL_FAULT_PIN = value;
        Serial.print("Digital Fault Pin = ");
        Serial.println(value);
      } else if (variableName == setCommands[2]) {
        ANALOG_FAULT_PIN = value;
        Serial.print("Analog Fault Pin = ");
        Serial.println(value);
      } else if (variableName == setCommands[3]) {
        INTERRUPT_PIN = value;
        Serial.print("Interrupt Pin = ");
        Serial.println(value);
      } else if (variableName == setCommands[4]) {
        state = value;
        Serial.print("State = ");
        Serial.print(value);
        if (state == 0) {
          Serial.println("Normal State = LOW and Fault State = HIGH");
        } else if (state == 1) {
          Serial.println("Normal State = HIGH and Fault State = LOW"); 
        } else {
          Serial.println("Invalid State! back to default");
          state = 0; 
          Serial.print("State = ");
          Serial.println(state);
          Serial.println("Normal State = LOW and Fault State = HIGH");
        }
      } else if (variableName == setCommands[5]) {
        maxFaultDuration = value;
        Serial.print("Maximum Fault Duration = ");
        Serial.println(maxFaultDuration);
      } else if (variableName == setCommands[6]) {
        universalDebounceDelay = value;
        Serial.print("Universal Debounce Delay = ");
        Serial.println(universalDebounceDelay);
      } else if (variableName == setCommands[7]) {
        incrementFactor = value;
        Serial.print("Increment Factor = ");
        Serial.println(incrementFactor);
      } else if (variableName == setCommands[8]) {
        decrementFactor = value;
        Serial.print("Decrement Factor = ");
        Serial.println(decrementFactor);
      } else if (variableName == setCommands[9]) {
        initialDuration = value;
        Serial.print("Initial Duration = ");
        Serial.println(initialDuration);
      } else {
        Serial.println("Invalid Command!");
      }
    } else if (command.startsWith("LIST")) {
      Serial.print("Trigger Pin = ");
      Serial.println(TRIGGER_PIN);

      Serial.print("Digital Fault Pin = ");
      Serial.println(DIGITAL_FAULT_PIN);

      Serial.print("Analog Fault Pin = ");
      Serial.println(ANALOG_FAULT_PIN);

      Serial.print("Interrupt Pin = ");
      Serial.println(INTERRUPT_PIN);

      Serial.print("State = ");
      Serial.print(state);
      if (state == 0) {
        Serial.println("Normal State = LOW and Fault State = HIGH");
      } else if (state == 1) {
        Serial.println("Normal State = HIGH and Fault State = LOW"); 
      }

      Serial.print("Maximum Fault Duration = ");
      Serial.println(maxFaultDuration);      

      Serial.print("Universal Debounce Delay = ");
      Serial.println(universalDebounceDelay);

      Serial.print("Increment Factor = ");
      Serial.println(incrementFactor);

      Serial.print("Decrement Factor = ");
      Serial.println(decrementFactor);

      Serial.print("Initial Duration = ");
      Serial.println(initialDuration);
    }
    else if (command.startsWith("execute")) {
      Serial.println("Under Development!");
    } 
    Serial.println("");
  }
}

void loop() {

  serial_console();

  // // Listen for the Trigger
  // while (true) {

  //   // Debounce Mechanism
  //   int triggerReading = digitalRead(TRIGGER_PIN);  // Read the Trigger Pin 
  //   if (triggerReading != lastTriggerState) {
  //     lastTriggerDebounceTime = millis();           // Reset the Trigger Debounce Timer 
  //   }

  //   if ((millis() - lastTriggerDebounceTime) > universalDebounceDelay) {    // Check if Debounce Delay for Trigger Pin has been passed
  //     if (triggerReading != triggerState) {
  //       triggerState = triggerReading;         // Set Trigger State to new Trigger Reading 

  //       // Fault Injection Function
  //       if (lastTriggerState == LOW) {        // Inject Fault if State changes from LOW to HIGH
  //         digital_incremental_interrupt_fault_injector(0, maxFaultDuration, 1);

  //         counter++;
  //         Serial.print("Fault Number: ");
  //         Serial.println(counter);
  //       }
  //       break;                                // Break out of the loop
  //     }
  //   }
  //   lastTriggerState = triggerReading;    // Update the last trigger state 
  // }
}