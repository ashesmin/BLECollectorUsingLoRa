main.exe : main.cpp ble_receiver.o
	g++ main.cpp ./lib/ble_receiver.o -lrt -lbluetooth -o main.exe

ble_receiver.o : ./lib/ble_receiver.cpp
	g++ -c -o ./lib/ble_receiver.o ./lib/ble_receiver.cpp -lrt -lbluetooth