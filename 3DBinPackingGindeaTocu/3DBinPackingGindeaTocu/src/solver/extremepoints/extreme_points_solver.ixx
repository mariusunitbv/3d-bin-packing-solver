module;
#include <pch.h>

export module extreme_points_solver;

export import solver;

export class ExtremePointsSolver : public ISolver {
   public:
    ExtremePointsSolver();
    void solveInternal() override;

    void cleanPoints();

   private:
    std::vector<Vector3> m_extremePoints;
};
