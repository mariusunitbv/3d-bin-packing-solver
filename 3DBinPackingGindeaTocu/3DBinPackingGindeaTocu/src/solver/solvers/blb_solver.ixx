module;
#include <pch.h>

export module blb_solver;

export import solver;

export class BLBSolver : public ISolver {
   public:
    BLBSolver();

    void solveInternal() override;
};
