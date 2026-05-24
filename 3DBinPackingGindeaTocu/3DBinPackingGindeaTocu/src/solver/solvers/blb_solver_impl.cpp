module;
#include <pch.h>

module blb_solver;

BLBSolver::BLBSolver() { m_stats.m_solverType = SOLVER_BLB; }

void BLBSolver::solveInternal() {
    for (const auto& box : m_model->getBoxes()) {
        bool placed = false;
        for (int z = 0; z <= (int)m_model->getContainerSize().z && !placed; z++) {
            for (int y = 0; y <= (int)m_model->getContainerSize().y && !placed; y++) {
                for (int x = 0; x <= (int)m_model->getContainerSize().x && !placed; x++) {
                    for (auto ori : kAllOrientations) {
                        if (!(box.m_allowedOrientations & ori)) {
                            continue;
                        }

                        Vector3 size = getOrientatedSize(box.m_size, ori);
                        PlacedBox candidate = {size, {(float)x, (float)y, (float)z}, box.m_color};
                        if (isValidPlacement(candidate)) {
                            m_solution.push_back(candidate);
                            placed = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}
