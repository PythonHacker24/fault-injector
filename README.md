# Fault Injector for ESP8266 and Arduino Boards 

![Intro](https://raw.githubusercontent.com/PythonHacker24/fault-injector/main/images/intro-image.png)

#### From the Workbench to the Battleground - Crafting a Fault Injection Attacker with an ESP8266 (or nearly any microcontrollers!)

## What is Fault Injection Attack? 

Fault Injection Attacks is an hardware attack that glitches a hardware circuit to behave in a way which is not intended and cause uncertain behaviour that would be useful for a hacker. 

This attack is useful in cases of jumping instructions inside a microprocessor or microcontrollers by gltiching the power supply to the chip. This maybe useful for escaping loops, bypassing authentication, etc. More destructive results can be obtained in cases of glitches that has undefined parameters set and may cause the whole electronic circuit to become unusuable due to deleting firmwares or bootloaders.

## Device Information

Since fault injections are so cool and need to be precise, a lot of work has to be put into doing it. It becomes expensive to carry out these attacks with dedicated tools and circuit. Hence, this project was developed, where efforts have been made to develop a device that can be crafted with inexpensive microcontrollers like ESP8266, which is developed with all required functionalities, or as much as possible. 



## Setting up the circuit

The code provided here is nearly enough to explain the wiring diagram of the circuit. Various parameters needs to be adjusted while developing the circuit, hence adjusting them before flashing the microcontroller is a good practice.

#### Requirements:
- ESP8266 (or ESP32, Arduino, etc. just adjust the code as per the device interfacing)
- Push Buttons
- Pull Down Registors (10K Ohms) 
- Workbench Components like Breadboard, Jumper Cables, Switching Mosfets are per requirements, Logic Analysers (for visualising digital fault injections), Oscilloscope (for digital and analog fault injections), etc. 
- Required Interfaces (for example, EMF induction circuit if EMF is to be used for fault injection)

#### Story of Pull Down Registors
While the initial phases of development of this project, it was found in a very hard way that pull down registors are required to keep the input pins grounded and at LOW state. Due to the loops and escape mechanism in the code, noise around the workspace can cause unintentional input triggering and escape loops causing wierd behaviour. Hence, it is recommended to use 10K Ohms pull down registors to every input pins to keep them at LOW state, wherever necessary. 

## Warning!

Note that this attack can unintentionally damage the circuit due to a lot of factors. Fault Injection attacks can be dangeruous and can cause permenant damage to the device. Hence, use this tool wih caution. 

The author of the project is not responsible for any damage caused by the project. So use it responsibly. 

## Development of the Project  

This project is under development process and the steps might vary from the documentation. Efforts are made to keep the documentation in par with the development process. Hence, refering to the code is useful while working with it to make sure things go as expected. 

