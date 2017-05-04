#!/bin/bash

clear

echo "Welcome Commander."
echo "The Evil Oranges must be stoped and holy bitcoin must be retrieved!!!"
echo "Good luck"
echo "You have 3 lifes"

g++ -c -std=c++11 src/*.cpp
g++ -o Star *.o -lSDL2 -lSDL2_image
./Star
