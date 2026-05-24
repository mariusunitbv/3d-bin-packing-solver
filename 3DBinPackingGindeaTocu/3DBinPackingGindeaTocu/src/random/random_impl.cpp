module;
#include <pch.h>

module random;

Random& Random::get() {
    static Random instance;
    return instance;
}

int Random::getInt(int min, int max) {
    std::uniform_int_distribution dist(min, max);
    return dist(Random::get().m_rng);
}

float Random::getFloat(float min, float max) {
    std::uniform_real_distribution dist(min, max);
    return dist(Random::get().m_rng);
}

std::mt19937& Random::rng() { return Random::get().m_rng; }
