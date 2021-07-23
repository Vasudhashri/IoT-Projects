Raspberry Pi controlled Arduino for making NFC payments with MQTT Protocol.

Authors: Vasudhashri Vijayaragavan
Introduction:
RFID (Radio Frequency Identification) is a technology that uses electromagnetic fields to identify objects in a contactless way; it is also called proximity identification. There are 2 elements in RFID communications: the RFID module (or reader/writer device) and an RFID card (or tag). The RFID module acts as the master and the card acts as the slave; this means the module queries the card and sends instructions to it. In a normal RFID communication, the RFID module is fixed and the user takes his card near it when he needs to start the interaction.
Hardware requirements for the whole project can be seen in the image rfid_1356_elementos_small.png. 

Step 1: Connecting the RFID/NFC module to Arduino
Setting up the hardware is very easy, just plug the XBee shield with the RFID/NFC module to Arduino. The jumpers in the XBee shield have to be set to XBEE position. See the picture RFID_1356_small.png. Now you can program the Arduino and communicate it with the RFID/NFC module using the serial port (Serial.read(), Serial.print()...). Remember you must unplug the XBee shield when uploading code to Arduino.

Step 2: Reading tags (read only cards)
In this part we show an example of the Arduino reading Mifare tags. We use the Auto Read mode. Arduino is waiting all the time, when a tag is detected, we read its code, and print the code over USB port. Run the codes, RFID-NFC_module/Reading tags (read only cards)/ of both Arduino and Raspberry Pi. 

Step 3 : Reading / writing Mifare tags
 The next example for this module is the writing/reading of tags.
Warning!!!
Don't write address 0, 3, 7, 11, 15, ... if you are not an advanced user!!! You could leave your tag unaccesable.
Each tag has some special blocks:

    Address 0 - . It contains the IC manufacturer data. Due to security and system requirements this block has read-only access after having been programmed by the IC manufacturer at production.
    Address 3, 7, 11... - Each sector has a sector trailer containing the secret keys A and B (optional), which return logical “0”s when read. Writing in them will change the access keys and access conditions!!!
Run the codes, RFID-NFC_module/Reading writing Mifare tags/ of both Arduino and Raspberry Pi.

Outputs: 
The USB output using the Arduino IDE serial port terminal can be seen in the image PantallaReadExample.png

The output of step 3 can be seen in the image PantallaWriteRead.png

This shield is tested and fully compatible with the following boards: 
Arduino Boards

    Arduino Uno

Raspberry Pi Boards

    Raspberry Pi
    Raspberry Pi (Model B+)
    Raspberry Pi 2
    Raspberry Pi 3
    
Fritzing Libraries:
RFID 13.56 MHz / NFC Module for Arduino can be connected to Arduino using a XBee shield and will communicate it using the Arduino serial port (UART).). All the fritzing libraries can be downloaded from https://www.cooking-hacks.com/index.html. 










Source: Cooking hacks tutorial documentation on RFID 13.56 MHz / NFC Module for Arduino and Raspberry Pi. 
