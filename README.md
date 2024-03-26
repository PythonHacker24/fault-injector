# Fault Injector for ESP8266 and Arduino Boards 

![Intro](https://raw.githubusercontent.com/PythonHacker24/fault-injector/main/images/intro-image.png)

#### From the Workbench to the Battleground - Crafting a Fault Injection Attacker with an ESP8266 (or nearly any microcontrollers!)

## What is Fault Injection Attack? 

Fault Injection Attacks is an hardware attack that glitches a hardware circuit to behave in a way which is not intended and cause uncertain behaviour that would be useful for a hacker. 

This attack is useful in cases of jumping instructions inside a microprocessor or microcontrollers by gltiching the power supply to the chip. This maybe useful for escaping loops, bypassing authentication, etc. More destructive results can be obtained in cases of glitches that has undefined parameters set and may cause the whole electronic circuit to become unusuable due to deleting firmwares or bootloaders.

## Device Information

Since fault injections are so cool and need to be precise, a lot of work has to be put into doing it. It becomes expensive to carry out these attacks with dedicated tools and circuit. Hence, this project was developed, where efforts have been made to develop a device that can be crafted with inexpensive microcontrollers like ESP8266, which is developed with all required functionalities, or as much as possible. 

### Features
#### Precise Time Adjustments 
This prototype with ESP8266 supports fault injections with microseconds of delay which can be adjusted. This is something that dependes upon the hardware used and it's support to causing delays.

#### Incremental and Decremental Fault Injections 
The time adjustments in fault injections is an important factor to consider and varies according to different circuits. Hence, moving step by step with increasing and decreasing fault injection periods can help find the exact glitch required for the attack. This helps in analysing the circuit behaviour of the circuit and developing stratergy of attack, or even hit the goal in time. 

#### Feedback Loop Support Via Interrupt Pin 
Triggering of Injections can be varied with feedbacks with external signals. This allows fault injections with precise timing into the electronic circuits upon certian events. This is included in the triggered fault injections, incremental fault injections as well as decremental fault injections. 

#### Step by Step Increments and Decrements 
Stepping up and down with push buttons to adjust voltage glitches helps in finding the exact spot for the fault injection attack with manual analysis. This feature allows users to use push button to increment or decrement (as per the functions setup) and step through various time delays to manually analyse the behaviour of the target.   

## Setting up the circuit

The code provided here is nearly enough to explain the wiring diagram of the circuit. Various parameters needs to be adjusted while developing the circuit, hence adjusting them before flashing the microcontroller is a good practice.

```
#define TRIGGER_PIN D1                // Trigger Input Pin (Pull Down Register is Required)
#define DIGITAL_FAULT_PIN D2          // Digital Output Pin
#define ANALOG_FAULT_PIN D3           // Analog Output Pin
#define INTERRUPT_PIN D4              // Interupt Input Pin (Pull Down Register is Required)
```

#### Requirements:
- ESP8266 (or ESP32, Arduino, etc. just adjust the code as per the device interfacing)
- Push Buttons
- Pull Down Registors (10K Ohms) 
- Workbench Components like Breadboard, Jumper Cables, Switching Mosfets are per requirements, Logic Analysers (for visualising digital fault injections), Oscilloscope (for digital and analog fault injections), etc. 
- Required Interfaces (for example, EMF induction circuit if EMF is to be used for fault injection)

## Warning!

Note that this attack can unintentionally damage the circuit due to a lot of factors. Fault Injection attacks can be dangeruous and can cause permenant damage to the device. Hence, use this tool wih caution. 

The author of the project is not responsible for any damage caused by the project. So use it responsibly. 

## Development of the Project  

This project is under development process and the steps might vary from the documentation. Efforts are made to keep the documentation in par with the development process. Hence, refering to the code is useful while working with it to make sure things go as expected. 

## License 

This project is licensed under the MIT License - see the [LICENSE](https://raw.githubusercontent.com/PythonHacker24/fault-injector/main/LICENSE) file for details.

![Project Under Development](https://raw.githubusercontent.com/PythonHacker24/fault-injector/main/images/project_under_development.png)
