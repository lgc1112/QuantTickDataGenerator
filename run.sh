clear
g++ -std=c++11 transaction_mgr.cpp logger.cpp test.cpp  live_tarding_data_simulator.cpp -o ./build/test -lpthread 
./build/test | tee  ./data/output.txt
