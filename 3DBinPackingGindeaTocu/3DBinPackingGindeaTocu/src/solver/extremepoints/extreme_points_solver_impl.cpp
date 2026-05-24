module;
#include <pch.h>

module extreme_points_solver;

ExtremePointsSolver::ExtremePointsSolver() { m_stats.m_solverType = SOLVER_EXTREME_POINTS; }

void ExtremePointsSolver::solveInternal() {
    const auto& containerSize = m_model->getContainerSize();

    auto& points = m_extremePoints;
    if (points.empty()) {
        points.push_back({0, 0, 0});
    }

    auto it = m_unplacedBoxes.begin();
    while (it != m_unplacedBoxes.end()) {
        int idx = *it;
        const auto& box = m_model->getBoxes()[idx];
        bool boxPlaced = false;

        std::sort(points.begin(), points.end(), [](const Vector3& a, const Vector3& b) {
            if (a.y != b.y) return a.y < b.y;
            if (a.z != b.z) return a.z < b.z;
            return a.x < b.x;
        });

        for (const auto& pt : points) {
            if (boxPlaced) break;

            for (auto ori : kAllOrientations) {
                if (!(box.m_allowedOrientations & ori)) continue;

                Vector3 size = getOrientatedSize(box.m_size, ori);

                PlacedBox candidate = {size, pt, box.m_color};
                if (!isValidPlacement(candidate)) continue;

                m_solution.push_back(candidate);
                it = m_unplacedBoxes.erase(it);
                boxPlaced = true;

                Vector3 p1 = {pt.x + size.x, pt.y, pt.z};
                Vector3 p2 = {pt.x, pt.y + size.y, pt.z};
                Vector3 p3 = {pt.x, pt.y, pt.z + size.z};

                auto addPoint = [&](Vector3 p) {
                    if (p.x >= containerSize.x) return;
                    if (p.y >= containerSize.y) return;
                    if (p.z >= containerSize.z) return;

                    for (const auto& existing : points) {
                        if (existing.x == p.x && existing.y == p.y && existing.z == p.z) return;
                    }
                    points.push_back(p);
                };

                addPoint(p1);
                addPoint(p2);
                addPoint(p3);

                cleanPoints();

                break;
            }
        }

        if (!boxPlaced) ++it;
    }
}

void ExtremePointsSolver::cleanPoints() {
    m_extremePoints.erase(
        std::remove_if(
            m_extremePoints.begin(), m_extremePoints.end(),
            [&](const Vector3& pt) {
                for (const auto& pb : m_solution) {
                    if (pt.x >= pb.m_position.x && pt.x < pb.m_position.x + pb.m_size.x &&
                        pt.y >= pb.m_position.y && pt.y < pb.m_position.y + pb.m_size.y &&
                        pt.z >= pb.m_position.z && pt.z < pb.m_position.z + pb.m_size.z) {
                        return true;
                    }
                }
                return false;
            }),
        m_extremePoints.end());
}
