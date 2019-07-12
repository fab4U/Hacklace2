# Hacklace2

For a detailed description of the project see

/documentation/hacklace_english.pdf


## How to compile the Hacklace2 firmware

We assume that the Arduino-IDE version 1.6.9 from www.arduino.cc is correctly installed on your computer.

1. Open the Hacklace2 repository in your web browser. Click on the green button „Clone or download“ and then on „Download ZIP“. 

2. Save the zip-file and then de-compress it, which will create a folder called „Hacklace2-master“.

3. Go to „Hacklace2-master“ and move the sub-folders „Hacklace_Games“, „HL_Hello_World“ and „Hacklace_Main“ to the folder for your Arduino projects.

4. Open the Arduino-IDE. In the „Sketch“ menu choose „Include Library -> Add .ZIP Library“ and select „<your location from step 2>/Hacklace2-master/libraries/Hacklace“.

5. Click on the „Open“ icon and open „Hacklace2 -> Hacklace_main“.

6. In the „Tools“ menu set the correct board and processor. For board select „Arduino Pro or Pro mini“ and as processor select „ATmega328 (3.3V, 8 MHz)“.

7. Compile and download to your Hacklace2.
