#!/bin/sh

node-gyp.sh configure

mkdir -p build/lib

CPP_DEFINES="-DCAFFE_VERSION=1.0.0-rc3 -DNDEBUG -DUSE_CUDNN -DUSE_OPENCV -DUSE_LEVELDB -DUSE_LMDB"
CPP_OPTS="-MMD -MP -pthread -fPIC -O2 -fmax-errors=5 -Wno-sign-compare  "   

g++ -c $CPP_OPTS  $CPP_DEFINES -I /home/richard/.node-gyp/7.1.0/include/node -I /home/richard/caffe/include -I /usr/local/cuda/include -I /home/richard/caffe/.build_release/src src/Caffelib.cpp -o build/Caffelib.o
if [ $? -ne 0 ] 
then 
	exit 
fi

g++ -L /usr/lib/x86_64-linux-gnu/ -shared -Wl,-soname,libcaffenet.so -o build/lib/libcaffenet.so build/Caffelib.o -lc
if [ $? -ne 0 ] 
then 
	exit 
fi

node-gyp.sh build
if [ $? -ne 0 ] 
then 
	exit 
fi

LD_LIBRARY_PATH=$LD_LIBRARY_PATH://home/richard/workspace/caffenet/lib:/lib:/home/richard/caffe/.build_release/lib/ node test/test.js


#doxygen src/lalg.dox 
#npm publish
#docker build --no-cache -t rcorbish/node-caffenet .
#docker push rcorbish/node-caffenet

