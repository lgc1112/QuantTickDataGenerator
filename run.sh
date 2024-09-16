# ../projects/nrc/code/3rdparty/x86_64-unknown-linux-gnu-gcc/bin/g++ -std=c++20 test.cpp  transaction_mgr.cpp  -o ./build/test -lpthread
clear
g++ -std=c++11 test.cpp transaction_mgr.cpp  logger.cpp -o ./build/test -lpthread
./build/test
