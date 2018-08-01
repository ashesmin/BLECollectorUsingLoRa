#!/bin/bash

PWD="$(pwd)"
cd lib/
BASE_DIR="$(pwd)"
BLE_DIR="$BASE_DIR/ble"
ARDUPI_DIR="$BASE_DIR/arduPi"
ARDUPIAPI_DIR="$BASE_DIR/arduPi-api"
LIBRARY_DIR="$BASE_DIR/libraries/arduPiLoRa"
cd ..
PROJECT_DIR="$(pwd)"

cd "$BLE_DIR"
file="./ble_receiver.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "ble_receiver.o -> purged"
		rm ./ble_receiver.o
	else
		echo "ble receiver already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling ble receiver... "
		g++ -c ble_receiver.cpp -o ble_receiver.o -lbluetooth
	fi
fi

cd "$BLE_DIR"
file="./ble_to_lora.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "ble_to_lora.o -> purged"
		rm ./ble_to_lora.o
	else
		echo "ble to lora already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling ble to lora... "
		g++ -c ble_to_lora.cpp -o ble_to_lora.o -I  "$LIBRARY_DIR" -I "$ARDUPI_DIR" -I .
	fi
fi

cd "$BLE_DIR"
file="./lora_to_eth.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "lora_to_eth.o -> purged"
		rm ./lora_to_eth.o
	else
		echo "lora to ethernet already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling lora to ethernet... "
		g++ -c lora_to_eth.cpp -o lora_to_eth.o
	fi
fi

cd "$BLE_DIR"
file="./util.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "util.o -> purged"
		rm ./util.o
	else
		echo "Utility already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling utility... "
		g++ -c util.cpp -o util.o -lbluetooth
	fi
fi

#compile arduPi
cd "$ARDUPI_DIR"
file="./arduPi.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "arduPi.o -> purged"
		rm ./arduPi.o
	else
		echo "arduPi already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling arduPi..."
		g++ -c arduPi.cpp -o arduPi.o
	fi
fi


#compile arduPi-api
cd "$ARDUPIAPI_DIR"
file="./arduPiUART.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "arduPiUART.o -> purged"
		rm ./arduPiUART.o
	else
		echo "arduPiUART already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling arduPiUART..."
		g++ -c arduPiUART.cpp -o arduPiUART.o
	fi
fi


file="./arduPiUtils.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "arduPiUtils.o -> purged"
		rm ./arduPiUtils.o
	else
		echo "arduPiUtils already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling arduPiUtils..."
		g++ -c arduPiUtils.cpp -o arduPiUtils.o
	fi
fi

file="./arduPiMultiprotocol.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "arduPiMultiprotocol.o -> purged"
		rm ./arduPiMultiprotocol.o
	else
		echo "arduPiMultiprotocol already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling arduPiMultiprotocol..."
		g++ -c arduPiMultiprotocol.cpp -o arduPiMultiprotocol.o
	fi
fi


#compile library
cd "$LIBRARY_DIR"
file="./arduPiLoRa.o"
if [ -e $file ]; then
	if [ "$1" == "-clean" ]; then
		echo "arduPiLoRa.o -> purged"
		rm ./arduPiLoRa.o
	else
		echo "arduPiLoRa already compiled"
	fi
else
	if [ "$1" != "-clean" ]; then
		echo "Compiling arduPiLoRa..."
		g++ -c arduPiLoRa.cpp \
			-I"$ARDUPIAPI_DIR" \
			-I"$ARDUPI_DIR" \
			-o arduPiLoRa.o
	fi
fi

sleep 1

cd "$PROJECT_DIR"
file="./$1"

if [ "$1" != "-clean" ]; then
if [ -e $file ]; then
if [ "$1" != "" ]; then
echo "Compiling PROJECT..."

g++ -lbluetooth -lrt -lpthread -lstdc++ "$1" \
				"$LIBRARY_DIR/arduPiLoRa.o" \
				"$ARDUPIAPI_DIR/arduPiUART.o" \
				"$ARDUPIAPI_DIR/arduPiUtils.o" \
				"$ARDUPIAPI_DIR/arduPiMultiprotocol.o" \
				"$ARDUPI_DIR/arduPi.o" \
				"$BLE_DIR/ble_receiver.o"\
				"$BLE_DIR/ble_to_lora.o"\
				"$BLE_DIR/lora_to_eth.o"\
				"$BLE_DIR/util.o"\
				-I"$ARDUPI_DIR"\
				-I"$ARDUPIAPI_DIR"\
				-I"$LIBRARY_DIR"\
				-I"$PROJECT_DIR"\
				-I"$BLE_DIR"\
				-o "$1_exe"
						else
						echo "---------------HELP------------------"
						echo "Compiling: ./mak.sh filetocompile.cpp"
						echo "Cleaning:  ./mak.sh -clean"
						echo "-------------------------------------"
						fi
						else
						echo "ERROR No such file or directory: $file"
						fi
						else
						echo "¡¡Spotless Kitchen!!"
						fi

						sleep 1

						exit 0

