#!/bin/bash
z80asm gpu-fill-screen.asm -o gpu-fill-screen.bin
z80asm main.asm -o main.bin
z80asm vectors.asm -o vectors.bin
java -jar ../../java-tools/rom-packer/rompacker.jar vectors.bin main.bin gpu-fill-screen.bin
echo "Trying to copy packed.bin to build directory..."
cp packed.bin ../build/bootrom.bin

