module;
#include <pch.h>

export module layer_based_solver;

export import solver;

export class LayerBasedSolver : public ISolver {
   public:
    LayerBasedSolver();

    void solveInternal() override;
};
