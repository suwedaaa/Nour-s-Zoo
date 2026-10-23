#pragma once
#include <random>
inline uint32_t esp_random() { static std::mt19937 random(std::random_device{}()); return random(); }
