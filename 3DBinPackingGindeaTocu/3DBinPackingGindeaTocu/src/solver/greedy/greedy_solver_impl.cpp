module;
#include <pch.h>

module greedy_solver;

GreedySolver::GreedySolver() { m_stats.m_solverType = SOLVER_GREEDY; }

void GreedySolver::solveInternal() {
    const auto& containerSize = m_model->getContainerSize();

    std::sort(m_unplacedBoxes.begin(), m_unplacedBoxes.end(), [&](int a, int b) {
        const auto& boxA = m_model->getBoxes()[a];
        const auto& boxB = m_model->getBoxes()[b];
        float volA = boxA.m_size.x * boxA.m_size.y * boxA.m_size.z;
        float volB = boxB.m_size.x * boxB.m_size.y * boxB.m_size.z;
        return volA > volB;
    });

    auto it = m_unplacedBoxes.begin();
    while (it != m_unplacedBoxes.end()) {
        int idx = *it;
        const auto& box = m_model->getBoxes()[idx];
        bool boxPlaced = false;

        for (float z = 0; z <= containerSize.z && !boxPlaced; z += 1.0f) {
            for (float y = 0; y <= containerSize.y && !boxPlaced; y += 1.0f) {
                for (float x = 0; x <= containerSize.x && !boxPlaced; x += 1.0f) {
                    for (auto ori : kAllOrientations) {
                        if (!(box.m_allowedOrientations & ori)) continue;

                        Vector3 size = getOrientatedSize(box.m_size, ori);

                        PlacedBox candidate = {size, {x, y, z}, box.m_color};
                        if (!isValidPlacement(candidate)) continue;

                        m_solution.push_back(candidate);
                        it = m_unplacedBoxes.erase(it);
                        boxPlaced = true;
                        break;
                    }
                }
            }
        }

        if (!boxPlaced) ++it;
    }
}
