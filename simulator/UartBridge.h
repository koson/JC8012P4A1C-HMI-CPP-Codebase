#pragma once
#include <stdbool.h>

class UartBridge {
public:
    static UartBridge& getInstance() {
        static UartBridge instance;
        return instance;
    }
    bool isConnected() const { return false; }
};
