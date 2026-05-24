module;
#include <pch.h>

module benchmark_timer;

BenchmarkTimer::BenchmarkTimer(const std::string& message) : m_message(message) {
    m_startTime = std::chrono::high_resolution_clock::now();
}

BenchmarkTimer::~BenchmarkTimer() {
    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime).count() /
        1000.0f;
    std::cout << m_message << " took " << duration << " seconds\n";
}
