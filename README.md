# BLECollectorUsingLoRa

This program consists of two parts : Sender and Receiver.

And The flow is as follows.

First, Sender gathers ble beacon packet.

After interval or If the number of scanned beacon is full,

sender send ble scanned data to receiver using lora.

Second, receiver receives ble scanned data.

receiver interprets payload and print contents.

and also it can treat other purposes.



#### 0. Prerequisite

1) Hardware
- Two raspberry pi 3 model B : One is sender, another is receiver. 

    Link : https://www.raspberrypi.org/products/raspberry-pi-3-model-b-plus/
    
- Two sx1272 lora shield

    Link : https://www.cooking-hacks.com/sx1272-lora-shield-for-raspberry-pi-868-mhz


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
