#!/bin/bash

g++ dwgra.cpp patch.cpp dgui.cpp crc32.cpp -o demo `allegro-config --libs`
