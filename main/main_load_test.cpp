// main/main_load_test.cpp
#include "load_test/LoadTester.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <host> <port> <threads> <msgs_per_thread>\n";
        return 1;
    }
    auto host  = std::string(argv[1]);
    auto port  = static_cast<uint16_t>(std::stoi(argv[2]));
    int  threads = std::stoi(argv[3]);
    int  msgs    = std::stoi(argv[4]);

    LoadTester tester(host, port, threads, msgs);
    tester.run();
    return 0;
}
