module;
#include <pch.h>

module layer_based_solver;

LayerBasedSolver::LayerBasedSolver() { m_stats.m_solverType = SOLVER_LAYERBASED; }

void LayerBasedSolver::solveInternal() {
    const auto& containerSize = m_model->getContainerSize();

    float currentY = 0;

    while (currentY < containerSize.y) {
        float layerHeight = 0;
        float currentX = 0;
        float currentZ = 0;
        float rowDepth = 0;
        bool anyPlaced = false;

        auto it = m_unplacedBoxes.begin();
        while (it != m_unplacedBoxes.end()) {
            int idx = *it;
            const auto& box = m_model->getBoxes()[idx];
            bool boxPlaced = false;

            for (auto ori : kAllOrientations) {
                if (!(box.m_allowedOrientations & ori)) continue;

                Vector3 size = getOrientatedSize(box.m_size, ori);

                if (layerHeight == 0) {
                    if (currentY + size.y > containerSize.y) continue;
                    layerHeight = size.y;
                }

                if (size.y > layerHeight) continue;

                if (currentX + size.x > containerSize.x) {
                    currentX = 0;
                    currentZ += rowDepth;
                    rowDepth = 0;
                }

                if (currentZ + size.z > containerSize.z) continue;
                if (currentY + size.y > containerSize.y) continue;

                m_solution.emplace_back(size, Vector3{currentX, currentY, currentZ}, box.m_color);
                it = m_unplacedBoxes.erase(it);
                currentX += size.x;
                rowDepth = std::max(rowDepth, size.z);
                anyPlaced = true;
                boxPlaced = true;
                break;
            }

            if (!boxPlaced) ++it;
        }

        if (!anyPlaced) break;
        currentY += layerHeight;
    }
}
