module;
#include <pch.h>

export module benchmark_timer;

export class BenchmarkTimer {
   public:
    BenchmarkTimer(const std::string& message = "");
    ~BenchmarkTimer();

   private:
    std::string m_message;
    std::chrono::high_resolution_clock::time_point m_startTime;
};
