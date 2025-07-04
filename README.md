# openDecabot
The easiest 3D printed robot you can assemble and program using Wemos D1 Mini Shields and Arduino IDE, powered by ESP8266. 

![Decabot robot on a Table](images/openDecabot_onTable.jpg)
![A hand holding a Decabot robot](images/openDecabot_onHand.jpg)

## Hardware
### ESP8266 microcontroler
Open Decabot® uses the ESP8266 controler, in the Wemos Mini form factor, wich allows to use small shields to expand its capacities. In the simplest version, this robot uses a LED matrix shield and a custom connection shield, besides the Wemos D1 mini. The custom shield can be build with a PCB perfboard, a few pin header connections, and some soldering skills. 

By using the Espressif ESP8266 platform, the open Decabot® is nativelly equiped with Wi-Fi capabilities, allowing great IoT experiences, such as remote control and telemetry. 

More information about Wemos platform and a list of main shields available can be found on www.wemos.cc .

### Servo Motors
The open Decabot® robot uses two 360 servo motors to drive its wheels. It's the simplest solution for small robots, since the servo motors didin't require H bridge circuits (they already have those inside!) and can be connected directly to the ESP8266 pins. 

### 3D printed body
You can assemble your Open Decabot® using 3D printed parts. The basic chassis uses only 8 3D printed pieces, and can be assembled using a few screws. The 3D files can be found on 3dPrint folder. 

### Simple AA batteries for power
Under the open Decabot® you can find a 4 x AA battery support, who can supply up to 6V directly to ESP board. This option simplifies the hardware, since the Wemos Mini board can hold the maxminum 6 volts, lowering the tension to 3 volts. Users can hack the project to use LiIon batteries, combined with the Wemos LiPo Battery Shield (https://www.wemos.cc/en/latest/d1_mini_shield/battery.html) for charging and step up the voltage to 5 volts. 

### Easy access to USB ports
There are two ports on the upper part of the robot who can be used to access the USB port (for firmware/Arduino sketch update), and other wiring and shield needs. 

![Decabot robot conneceted using USB cable to a computer](images/usbConnection_openDecabotMini.jpg)

### Bill of Materials - BOM
- 1 x Wemos D1 Mini ESP8266 controller
- 2 x Servo 5V 360
- 1 x Wemos LED Matrix Shield
- 1 x custom PCB board (check wiki how to build one) or 1 x Decabot Mini Shield PCB (see /decabotMiniShield folder)
- 1 x battery support for 4 x AA
- 1 x acrylic frontface + 2 hallen 3mm screws (optional)
- 7 x screws 3mm, 12mm long
- 2 x rubber o-ring for wheels, size 2" or 50mm (ref: DN50)
- 4 x AA batteries

### Decabot Mini Shield
Wemos platform does not have a simple shield board to connect servos, so we provided a PCB project, using an open-hardware license, to order custom PCBs to your robots. This board has a few components, but it's goal is just to simplify the connection between the controller and the motors. The components are:
- 1 x Buzzer on D8 pin
- 2 x Female pin header 1x8 for Wemos connection
- 2 x Female pin header 1x4 (or 1x5) for I2C, gyroscope or xDOF modules
- 6 x Male pin header 1x3 for servos
- 1 x power switch
- 1 x battery connection (optional)

![Decabot Mini Shield front face](decabotMiniShield/front_PCB_decabotMiniShieldV1.png)
![Decabot Mini Shield back face](decabotMiniShield/back_PCB_decabotMiniShieldV1.png)

A homemade PDB board can be assembled just to connect the main components to the Wemos board: Battery to 5V pin, and servos to D0 and D6 pins, as shown below:

![Homemade Decabot Mini PCB](decabotMiniShield/openDecabot_homemadePcb.png)

## 3D project source
The 3D source project was all designed on Tinkercad (yes!), and can be forked using the link below:
https://www.tinkercad.com/things/dYy4XWihfYu-opendecabot-mini-v10

![Decabot robot on Tinkercad](images/openDecabot_3d.png)

## Decabot® Trade Mark
Besides this project is open source, the Decabot® name is a registered trademark from Decano Kits Company, and should not be used without proper consent. 
