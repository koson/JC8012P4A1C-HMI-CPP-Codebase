#pragma once
#include <stdbool.h>

class VerificationEngine {
public:
    static VerificationEngine& getInstance() {
        static VerificationEngine instance;
        return instance;
    }
};
