#!/bin/sh

node-gyp.sh configure

mkdir -p build/lib

g++ -c -fPIC -std=gnu++0x -fmax-errors=5 -Wno-sign-compare -Wall -I /home/richard/.node-gyp/7.1.0/include/node -I /home/richard/caffe/include -I /usr/local/cuda/include -I /home/richard/caffe/.build_release/src src/Caffelib.cpp -o build/Caffelib.o

g++ -L /usr/lib/x86_64-linux-gnu/ -shared -Wl,-soname,libcaffenet.so -o build/lib/libcaffenet.so build/Caffelib.o -lc

node-gyp.sh build

LD_LIBRARY_PATH=$LD_LIBRARY_PATH://home/richard/workspace/caffenet/lib:/lib:/home/richard/caffe/.build_release/lib/ node test/test.js


#doxygen src/lalg.dox 
#npm publish
#docker build --no-cache -t rcorbish/node-caffenet .
#docker push rcorbish/node-caffenet

