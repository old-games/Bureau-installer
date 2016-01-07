#!/bin/bash

g++ dwgra.cpp patch.cpp dgui.cpp crc32.cpp -o demo `allegro-config --frameworks`
fixbundle demo -e -16 stuff/d_16.pcx -32 stuff/d_32.pcx -48 stuff/d_48.pcx -128 stuff/d_128.pcx
