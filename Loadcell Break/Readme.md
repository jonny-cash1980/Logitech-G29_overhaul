# Introduction:
This Mod is to update the regular potentioneter-based break input from the Logitech G92x based ffb wheels. I own an g920, so all instructions will generally base on this wheel.

After some research a combination of a loadcell with a driverboard (HX711) and a regular ESP32 should do most of the trick.
Most of this project will be based of the research of **europer** who did a great job 6 years ago. When compiling his program i got errors and so i thougt about giving the old approach an overhaul by reviewing the code to be readable and easy and most important to change the initialization prozess from bluetooth serial to an lightweigt webserver baser approach.

## Project steps

1. Select **Hardware/Firmware** (there is enoght around to not start with a blank sheet of paper)
2. Wireing the Hardware
3. Writing the firmware
   - Webserver with config page
   - Data inpt from X711 board
   - "Calculation"
   - Optput in
     - Joycon driver via USB for PC (reduce latency and no need for linearization)
     - PWM output for direct connection to G920 harnest
5.  Beers...
-----------------------------
## 1. Select **Hardware/Firmware**

### Hardware
#### Loadcell sensor and driver
Regular scale sensor, often sold directly with driver-board. (Ebay in germany did had good offerings, Ali... should have similar ones) Like in europer's setup i will use 2 sensors, knowing this is not realy necessary but the stl parts were designed for this so i gave it a try

#### 
