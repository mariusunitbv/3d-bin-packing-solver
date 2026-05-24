module;
#include <pch.h>

export module greedy_solver;

export import solver;

export class GreedySolver : public ISolver {
   public:
    GreedySolver();
    void solveInternal() override;
};
