/* 
  Fault Injector for ESP8266 (nodeMCU) and Arduino (ESP8266 is preferred due to it's clock speed)

  Author: Aditya Patil <Hack the Planet!>

  Fault Injection Attack can be used for attacking electronic circuits in various ways. This is a Fault Injection toolkit for hunting fault injection vulnerabilities in circuits. 
  Requirements:
    Push Button - Trigger Button 
    Jumper Wires 
    Pull Down Resistors - 10K Ohms for Fault Pin 

  Parameters can be varied in the code for adjusting pinouts. 
  Important Prameters: 
    TRIGGER_PIN - Pin for Push Button Input (Fault Trigger)
    DIGITAL_FAULT_PIN - Pin for Digital Fault Injection 
    ANALOG_FAULT_PIN - Pin for Analog Fault Injection 
    INTERRUPT_PIN - Interrupt Input Pin
*/

/*
Notes:

1. Add a analyser built in into the fault injector. This would be interfaced with the feedback interrupt of the glitching trigger, which would trigger in certian specific event like a hex value from the digital line passing. 
  It's like a logic analyser so a data decryption and interpretion must be considered.
2. For adjustable voltage on target, PWN signal generation can be used to hook with an voltage control module to adjust the voltage from serial console. This would be helpful while designing a fault injection board.
*/

#include <ESP8266WiFi.h>

uint8_t TRIGGER_PIN = D1;                // Trigger Input Pin (Pull Down Resistor is Required)
uint8_t DIGITAL_FAULT_PIN = D2;          // Digital Output Pin
uint8_t ANALOG_FAULT_PIN = D3;           // Analog Output Pin
uint8_t INTERRUPT_PIN = D8;              // Interupt Input Pin (Pull Down Resistor is Required)

// Default Define Normal State and Fault State 
byte normalState = 0x0;
byte faultState = 0x1; 

// Parameters Fault Duration
int faultDuration = 0;
int maxFaultDuration = 100; 

int incrementFactor = 1;
int decrementFactor = 1;
int initialDuration = 0; 

// Trigger Debounce Mechanism Declarations 
int triggerState;             // Current State of the Trigger
int lastTriggerState = LOW;   // Initialise last trigger state to LOW

int interruptState;
int lastInterruptState = LOW;

// Stepping Debounce Mechanism Declarations for Incremental and Decremental 
int stepperState;
int lastStepperState = LOW;

unsigned long lastInterruptDebounceTime = 0; 
unsigned long lastStepperDebounceTime = 0;
unsigned long lastTriggerDebounceTime = 0;    // Last time when the Trigger was pressed 
int universalDebounceDelay = 10;              // Debounce Delay time for all buttons 

int counter = 0;   
int state = 0;

int consoleMode = 1; 

const char* setCommands[] = {
  "ST",                             // Set State 
  "MFD",                            // Set Maximum Fault Duration
  "UDD",                            // Set Universal Debounce Delay
  "IF",                             // Set Increment Factor 
  "DF",                             // Set Decrement Factor 
  "ID",                             // Set Initial Duration
};

const char* attackCommands[] = {
  "glitch",
  "single",
  "interrupt",
  "interruptINC",
  "interruptDCR",
  "interruptINCSU",
  "interruptDCRSD",
};

const char* setInfo[] = {
  "Set State (0 for Fault Pin to be LOW in normal state and 1 for Fault Pin to be HIGH in normal state)",
  "Maximum Fault Duration (Maximum Fault Duration to permit for incremental stepping)",
  "Universal Debounce Delay (Debounce Period for buttons in the circuit",
  "Increment Factor (Time Period to increment in incremental stepping)",
  "Decrement Factor (Time Period to decrement in decremental stepping", 
  "Initial Duration (Initial Duration for incremental and decremental stepping)",
};

const char* attackInfo[] = {
  "glitch - Glitch Mode for attacking",
  "single - Glitch once (Usage: attack glitch single <interval>)",
  "interrupt - Glitch with Interrupt Mode, INTERRUPT_PIN takes Interrupt Signal (Usage: attack glitch interrupt)",
  "interruptINC - Increment time with increment parameter, INTERRUPT_PIN takes Interrupt Signal (Usage: attack glitch interruptINC <interval>)",
  "interruptDCR - Decrement time with decrement parameter, INTERRUPT_PIN takes Interrupt Signal (Usage: attack glitch interruptDCR <interval>)",
  "interruptINCSU - Increment time with increment parameter with stepping up, INTERRUPT_PIN takes Stepping Signal (Usage: attack glitch interruptINCSU)",
  "interruptDCRSD - Decrement time with decrement parameter with stepping down, INTERRUPT_PIN takes Stepping Signal (Usage: attack glitch interrputDCCRSD)",
};

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

  state_declaration(state);

  digitalWrite(DIGITAL_FAULT_PIN, normalState);        // Fault Pin State to HIGH
  
  Serial.begin(9600); 
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
  if (fault_duration < 0) {
    Serial.println("Fault Duration cannot be negative!");
  } else {
    while (true) {
      int interruptReading = digitalRead(INTERRUPT_PIN);
      if (interruptReading != lastInterruptState) {
        lastInterruptDebounceTime = millis();
      }
      if ((millis() - lastInterruptDebounceTime) > universalDebounceDelay) {
        if (interruptReading != interruptState) {
          interruptState = interruptReading;

          if (lastInterruptState == LOW) {
            digital_fault_injector(fault_duration);
            Serial.println("Fault Injected on Interrupt!");
          }
          break;
        }
      }
      lastInterruptState = interruptReading; 
    }
  }
}

// Variable Incremental Fault Injector Function
void digital_incremental_fault_injector(int initialDuration, int maxFaultDuration, int incrementFactor, int delayMicro) {
  if (initialDuration < 0 || maxFaultDuration < 0 || incrementFactor < 0 || delayMicro < 0) {
    Serial.println("Parameters cannot be negative!");
  } else {
    for (int downTime = 0; downTime < maxFaultDuration; downTime = downTime + incrementFactor) {
      digital_fault_injector(downTime);
      delayMicroseconds(delayMicro);
    }
  }
}

// Variable Decremental Fault Injector Funcrion
void digital_decremental_fault_injector(int initialDuration, int maxFaultDuration, int decrementFactor, int delayMicro) {
  if (initialDuration < 0 || maxFaultDuration < 0 || decrementFactor < 0 || delayMicro < 0) {
    Serial.println("Parameters cannot be negative!");
  } else {  
    for (int downTime = maxFaultDuration; downTime > initialDuration; downTime = downTime - decrementFactor) {
      digital_fault_injector(downTime);
      delayMicroseconds(delayMicro);
    }
  }
}

// Stepping up and down allows user to step up or down the voltage by some factor with buttons. 
// This is particularly helpful for debugging manually and finding the correct time duration for fault injection
// Here, D4 is now the stepping function
void step_up_digital_incremental_interrupt_fault_injector(int initialDuration, int incrementFactor) {
  if (initialDuration < 0 || incrementFactor < 0) {
    Serial.println("Parameters cannot be negative!");
  } else {
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
}

void step_down_digital_decremental_interrupt_fault_injector(int initialDuration, int decrementFactor) {
  if (initialDuration < 0 || decrementFactor < 0) {
    Serial.println("Parameters cannot be negative!");
  } else {
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
}

void serial_console() {

  Serial.flush();

  if (Serial.available()) {                         // Check the availability of Serial Console 
    String command = Serial.readStringUntil('\n');  // Take commands from the Serial Console 

    // Parameter Setting Section 
    if (command.startsWith("set")) {
      String variableName = command.substring(4, command.indexOf(' ', 4));
      String valueString = command.substring(command.lastIndexOf(' ') + 1);
      int value = valueString.toInt();

      if (variableName == setCommands[0]) {
        state = value;
        Serial.print("State = ");
        Serial.println(value);
        if (state == 0) {
          Serial.println(" - Normal State = LOW and Fault State = HIGH");
        } else if (state == 1) {
          Serial.println(" - Normal State = HIGH and Fault State = LOW"); 
        } else {
          Serial.println("Invalid State! back to default");
          state = 0; 
          Serial.print("State = ");
          Serial.println(state);
          Serial.println("Normal State = LOW and Fault State = HIGH");
        }
      } else if (variableName == setCommands[1]) {
        maxFaultDuration = value;
        Serial.print("Maximum Fault Duration = ");
        Serial.println(maxFaultDuration);
      } else if (variableName == setCommands[2]) {
        universalDebounceDelay = value;
        Serial.print("Universal Debounce Delay = ");
        Serial.println(universalDebounceDelay);
      } else if (variableName == setCommands[3]) {
        incrementFactor = value;
        Serial.print("Increment Factor = ");
        Serial.println(incrementFactor);
      } else if (variableName == setCommands[4]) {
        decrementFactor = value;
        Serial.print("Decrement Factor = ");
        Serial.println(decrementFactor);
      } else if (variableName == setCommands[5]) {
        initialDuration = value;
        Serial.print("Initial Duration = ");
        Serial.println(initialDuration);
      } else {
        Serial.println("Invalid Command!");
      }
    } else if (command.startsWith("attack")) {
      int spaceIndex1 = command.indexOf(' ');
      int spaceIndex2 = command.indexOf(' ', spaceIndex1 + 1);
      
      String firstParam = command.substring(spaceIndex1 + 1, spaceIndex2);
      int spaceIndex3 = command.indexOf(' ', spaceIndex2 + 1);
      
      String secondParam = command.substring(spaceIndex2 + 1, spaceIndex3);
      String thirdParam = command.substring(spaceIndex3 + 1);

      if (firstParam == attackCommands[0]) {
        if (secondParam == attackCommands[1]) {
          Serial.println("Glitched Single!");
          digital_fault_injector(thirdParam.toInt());
        } else if (secondParam == attackCommands[2]) {
          Serial.println("Glitched Interrupted!");
          digital_interrupted_fault_injector(thirdParam.toInt());
        } else if (secondParam == attackCommands[3]) {
          digital_incremental_fault_injector(initialDuration, maxFaultDuration, incrementFactor, thirdParam.toInt());
        } else if (secondParam == attackCommands[4]) {
          digital_decremental_fault_injector(initialDuration, maxFaultDuration, decrementFactor, thirdParam.toInt());
        } else if (secondParam == attackCommands[5]) {
          step_up_digital_incremental_interrupt_fault_injector(initialDuration, incrementFactor); 
        } else if (secondParam == attackCommands[6]) {
          step_down_digital_decremental_interrupt_fault_injector(initialDuration, decrementFactor); 
        } else {
          Serial.println("Invalid Command!");
        }
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
        Serial.println(" - Normal State = LOW and Fault State = HIGH");
      } else if (state == 1) {
        Serial.println(" - Normal State = HIGH and Fault State = LOW"); 
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

    } else if (command.startsWith("HELP")) {
      Serial.println("Fault Injector - A Fault Injection Toolkit\n");
      Serial.println("- All time values are in Mircoseconds\n");
      Serial.println("Configuration Commands\n");
      for (int index = 0; index < sizeof(setCommands) / sizeof(setCommands[0]); index++) {
        Serial.print(setCommands[index]);
        Serial.print(" - ");
        Serial.println(setInfo[index]);
      }
      Serial.println("\nAttack Commands\n");
      for (int index = 0; index < sizeof(attackCommands) / sizeof(setCommands[0]); index++) {
        Serial.print(attackCommands[index]);
        Serial.print(" - ");
        Serial.println(attackInfo[index]);
      }
    } else if (command.startsWith("EXIT")) {
      Serial.println("Exiting Serial Console");
    }
    Serial.println("");
  }
}

void loop() {
  if (consoleMode == 1) {
    serial_console();
  }
}
