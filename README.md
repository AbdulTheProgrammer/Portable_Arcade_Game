# Portable_Arcade_Game
Space Invaders recreated for the MSP430 microcontroller

This project was a product of an online massively open online course(MOOC) held by the University of Austin Texas
completed outside of school. The course, titled Embedded Systems, taught me the fundamentals of embedded
systems programming using C and how to interact with various hardware peripherals. This project also introduced me device drivers and I was able to make use of this knowledge to make simple drivers to control the DAC,ADC and a series of LEDS. The main requirement of
the project was to make a Space Invaders Game using the MSP430 microcontroller. However, I decided
to go far beyond these basic guidelines by increasing the complexity of the game and developing hardware around
it.

How it works: 
The main program uses a SPI display driver to render BMP images corresponding to game object to the Nokia LCD monitor. The main game loop is primarily responsible to detecting object collisions and rerendering the display. An interrupt fires 30 times per second and updates various game state values and sets a control flag indicating that a state change has occurred, which is used to communicate with the main task. This flag is then reset once the LCD monitor has been rerendered. 

Various sounds are also output to the ADC whenever the corresponding event occurs within the game (i.e. player shoots a missle, enemy shoots a laser etc). These sound clips come from an array of DAC output values which are used by the 4 bit DAC to produce a rough sinusoidal signal; different sound clips produce different frequency waves. Output to the DAC is controlled by a finite state machine, which has three states: not playing, level ended and playing. When a sound clip needs to be played the corresponding function pointer is loaded onto a circular queue(the sound buffer). A high priority interrupt, which executes at a rate of 11.025 kHz, acts as the main FSM controller. This FSM controller works as follows: 

- If the current state is not playing, the FSM controller transtions into the playing state and executes the dequeue operation on the circular queue, which returns a function pointer. Once this function pointer is dereferenced, it initiates playing of the corresponding sound clip by outputing the first DAC value. 

- Subsequent DAC values are continously output to the DAC until the entire sound is played to the player at which point, the FSM controller transitions to the idle state or dequeues another function pointer to play another sound. 

- Whenever a level ends, the FSM controller transitions to the level end state, which immediately stops outputting to the DAC and resets the sound buffer to reset the game.    

A circular queue is used for this application as it has a O(1) enqueue and dequeue operations and is relatively easy to implement in an embedded system. This constant run time is important as code written in an interrupt must always be short and bounded. 

### Software: 
Embedded C
### Hardware/Protocols: 
SPI, UART, I2C, MSP430, Custom-built PCB, various electrical components sourced from DigiKey
### Tools: 
EagleCAD, ViewMate, soldering iron, hand drill, digital multimeter

![Portable_Arcade_Game](/Images/16237697_10206562338056006_1635670807_n.jpg)

![Portable_Arcade_Game](/Images/16237222_10206562333535893_766669284_n.jpg)
