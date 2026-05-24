module;
#include <pch.h>

export module hillclimbing_solver;

export import solver;

import individual;

export class HillClimbingSolver : public ISolver {
   public:
    HillClimbingSolver();

    void initializeParameters() override;
    void solveInternal() override;

   private:
    int m_maxIterations;

    Individual m_bestIndividual;
};
