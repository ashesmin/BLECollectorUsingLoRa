# BLECollectorUsingLoRa

#### 0. Prerequisite

1) Hardware
- Two raspberry pi 3 model B : One is sender, another is receiver. 
    Link : https://www.raspberrypi.org/products/raspberry-pi-3-model-b-plus/
- Two 

#### 1. Compile

<code> $ chmod 755 ./cook.sh</code>

<code> $ ./cook.sh -clean </code>

1) Sender

<code> $ ./cook.sh [sender.cpp] </code>


2) Receiver

<code> $ ./cook.sh [receiver.cpp] </code>


#### 2. Run

1) Sender

<code>$ ./sender.cpp_exe [max_packet_num] [interval(seconds)] </code>

> $ ./sender.cpp_exe 20 1

2) Receiver

<code>$ ./receiver.cpp_exe </code>
