module;
#include <pch.h>

module solver;

import random;

import blb_solver;
import layer_based_solver;
import extreme_points_solver;
import greedy_solver;
import genetic_solver;
import hillclimbing_solver;

std::unique_ptr<ISolver> ISolver::create(SolverType_t type) {
    switch (type) {
        case SOLVER_BLB:
            return std::make_unique<BLBSolver>();
        case SOLVER_LAYERBASED:
            return std::make_unique<LayerBasedSolver>();
        case SOLVER_EXTREME_POINTS:
            return std::make_unique<ExtremePointsSolver>();
        case SOLVER_GREEDY:
            return std::make_unique<GreedySolver>();
        case SOLVER_GENETIC:
            return std::make_unique<GeneticSolver>();
        case SOLVER_HILLCLIMBING:
            return std::make_unique<HillClimbingSolver>();
        default:
            throw std::invalid_argument("Unknown solver type");
    }
}

void ISolver::setModel(ProblemModel* model) {
    m_model = model;
    m_stats.m_totalBoxes = (int)model->getBoxes().size();

    m_unplacedBoxes.resize(m_stats.m_totalBoxes);
    std::iota(m_unplacedBoxes.begin(), m_unplacedBoxes.end(), 0);
}

void ISolver::setParameter(const std::string& key, const std::string& value) {
    m_parameters[key] = value;
}

const std::vector<PlacedBox>& ISolver::getSolution() const { return m_solution; }

const SolverStats& ISolver::getStats() const { return m_stats; }

void ISolver::setExistingSolution(const std::vector<PlacedBox>& solution) { m_solution = solution; }

void ISolver::setDecoderType(SolverType_t decoder) { m_decoderType = decoder; }

void ISolver::solve() {
    const auto now = std::chrono::high_resolution_clock::now();
    solveInternal();
    const auto end = std::chrono::high_resolution_clock::now();
    m_stats.m_timeTaken =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count() / 1000.0f;

    m_stats.m_boxesPlaced = (int)m_solution.size();
    float usedVolume = 0;
    for (const auto& pb : m_solution) usedVolume += pb.m_size.x * pb.m_size.y * pb.m_size.z;

    const auto& c = m_model->getContainerSize();
    float containerVolume = c.x * c.y * c.z;

    m_stats.m_volumeUtilization =
        containerVolume > 0 ? (usedVolume / containerVolume) * 100.0f : 0.0f;
}

Individual ISolver::createRandomIndividual() {
    Individual ind;

    const auto& boxes = m_model->getBoxes();
    std::vector<uint32_t> boxIndices(boxes.size());
    std::iota(boxIndices.begin(), boxIndices.end(), 0);

    std::shuffle(boxIndices.begin(), boxIndices.end(), Random::rng());

    ind.m_chromosomes.reserve(boxes.size());
    for (uint32_t boxIndex : boxIndices) {
        ind.m_chromosomes.emplace_back(boxIndex, getRandomOrientation(boxes[boxIndex]));
    }
    ind.m_fitness = evaluateFitness(ind);

    return ind;
}

bool ISolver::isValidPlacement(const PlacedBox& candidate) const {
    const auto& containerSize = m_model->getContainerSize();

    if (candidate.m_position.x < 0 || candidate.m_position.y < 0 || candidate.m_position.z < 0 ||
        candidate.m_position.x + candidate.m_size.x > containerSize.x ||
        candidate.m_position.y + candidate.m_size.y > containerSize.y ||
        candidate.m_position.z + candidate.m_size.z > containerSize.z) {
        return false;
    }

    for (const auto& placed : m_solution) {
        if (overlaps(candidate, placed)) {
            return false;
        }
    }

    return true;
}

Orientation ISolver::getRandomOrientation(const Box& box) const {
    std::vector<Orientation> validOrientations;

    for (auto o : kAllOrientations) {
        if ((box.m_allowedOrientations & o) != 0) {
            validOrientations.push_back(o);
        }
    }

    if (validOrientations.empty()) {
        return ORIENTATION_WLH;
    }

    return validOrientations[Random::getInt(0, (int)validOrientations.size() - 1)];
}

std::vector<PlacedBox> ISolver::decodeIndividual(const Individual& ind) const {
    switch (m_decoderType) {
        case SOLVER_GREEDY:
            return decodeGreedy(ind);
        case SOLVER_LAYERBASED:
            return decodeLayerBased(ind);
        case SOLVER_BLB:
            return decodeBLB(ind);
        case SOLVER_EXTREME_POINTS:
            return decodeExtremePoints(ind);
        default:
            return decodeLayerBased(ind);
    }
}

double ISolver::evaluateFitness(const Individual& ind) const {
    std::vector<PlacedBox> simulatedSolution = decodeIndividual(ind);
    if (simulatedSolution.empty()) {
        return 0.0;
    }

    size_t totalBoxesInProblem = m_model->getBoxes().size();
    if (totalBoxesInProblem == 0) {
        return 0.0;
    }

    return (static_cast<double>(simulatedSolution.size()) / totalBoxesInProblem) * 100.0;
}

void ISolver::mutate(Individual& ind) {
    size_t idx1 = Random::getInt(0, (int)ind.m_chromosomes.size() - 1);
    size_t idx2 = Random::getInt(0, (int)ind.m_chromosomes.size() - 1);

    if (idx1 != idx2) {
        std::swap(ind.m_chromosomes[idx1], ind.m_chromosomes[idx2]);
    }
}

std::vector<PlacedBox> ISolver::decodeBLB(const Individual& ind) const {
    std::vector<PlacedBox> placedBoxes;
    placedBoxes.reserve(ind.m_chromosomes.size());

    const auto& containerSize = m_model->getContainerSize();
    const auto& allBoxes = m_model->getBoxes();

    auto isValid = [&](const Vector3& pos, const Vector3& size) -> bool {
        if (pos.x + size.x > containerSize.x || pos.y + size.y > containerSize.y ||
            pos.z + size.z > containerSize.z)
            return false;

        for (const auto& pb : placedBoxes) {
            if (pos.x < pb.m_position.x + pb.m_size.x && pos.x + size.x > pb.m_position.x &&
                pos.y < pb.m_position.y + pb.m_size.y && pos.y + size.y > pb.m_position.y &&
                pos.z < pb.m_position.z + pb.m_size.z && pos.z + size.z > pb.m_position.z) {
                return false;
            }
        }
        return true;
    };

    for (const auto& gene : ind.m_chromosomes) {
        const auto& box = allBoxes[gene.m_boxIndex];
        Vector3 size = getOrientatedSize(box.m_size, gene.m_orientation);

        bool placed = false;

        std::vector<Vector3> candidatePoints = {{0, 0, 0}};
        for (const auto& pb : placedBoxes) {
            candidatePoints.push_back(
                {pb.m_position.x + pb.m_size.x, pb.m_position.y, pb.m_position.z});
            candidatePoints.push_back(
                {pb.m_position.x, pb.m_position.y, pb.m_position.z + pb.m_size.z});
            candidatePoints.push_back(
                {pb.m_position.x, pb.m_position.y + pb.m_size.y, pb.m_position.z});
        }

        std::sort(candidatePoints.begin(), candidatePoints.end(),
                  [](const Vector3& a, const Vector3& b) {
                      if (a.y != b.y) return a.y < b.y;
                      if (a.z != b.z) return a.z < b.z;
                      return a.x < b.x;
                  });

        for (const auto& pt : candidatePoints) {
            if (isValid(pt, size)) {
                placedBoxes.emplace_back(size, pt, box.m_color);
                placed = true;
                break;
            }
        }
    }

    return placedBoxes;
}

std::vector<PlacedBox> ISolver::decodeLayerBased(const Individual& ind) const {
    std::vector<PlacedBox> placedBoxes;
    placedBoxes.reserve(ind.m_chromosomes.size());

    const auto& containerSize = m_model->getContainerSize();
    const auto& allBoxes = m_model->getBoxes();

    float currentY = 0.0f;
    float currentX = 0.0f;
    float currentZ = 0.0f;
    float layerHeight = 0.0f;
    float rowDepth = 0.0f;

    for (const auto& gene : ind.m_chromosomes) {
        const auto& box = allBoxes[gene.m_boxIndex];
        Vector3 size = getOrientatedSize(box.m_size, gene.m_orientation);

        if (size.x > containerSize.x || size.y > containerSize.y || size.z > containerSize.z) {
            continue;
        }

        if (layerHeight == 0.0f) {
            if (currentY + size.y > containerSize.y) break;
            layerHeight = size.y;
        }

        if (currentX + size.x > containerSize.x) {
            currentX = 0.0f;
            currentZ += rowDepth;
            rowDepth = 0.0f;
        }

        if (currentZ + size.z > containerSize.z || size.y > layerHeight) {
            currentY += layerHeight;
            currentX = 0.0f;
            currentZ = 0.0f;
            rowDepth = 0.0f;
            layerHeight = size.y;
        }

        if (currentX + size.x > containerSize.x || currentZ + size.z > containerSize.z ||
            currentY + size.y > containerSize.y) {
            continue;
        }

        placedBoxes.emplace_back(size, Vector3{currentX, currentY, currentZ}, box.m_color);

        currentX += size.x;
        rowDepth = std::max(rowDepth, size.z);
    }

    return placedBoxes;
}

std::vector<PlacedBox> ISolver::decodeExtremePoints(const Individual& ind) const {
    std::vector<PlacedBox> placedBoxes;
    placedBoxes.reserve(ind.m_chromosomes.size());

    const auto& containerSize = m_model->getContainerSize();
    const auto& allBoxes = m_model->getBoxes();

    std::vector<Vector3> epList = {{0, 0, 0}};

    auto isValid = [&](const Vector3& pos, const Vector3& size) -> bool {
        if (pos.x + size.x > containerSize.x || pos.y + size.y > containerSize.y ||
            pos.z + size.z > containerSize.z)
            return false;

        for (const auto& pb : placedBoxes) {
            if (pos.x < pb.m_position.x + pb.m_size.x && pos.x + size.x > pb.m_position.x &&
                pos.y < pb.m_position.y + pb.m_size.y && pos.y + size.y > pb.m_position.y &&
                pos.z < pb.m_position.z + pb.m_size.z && pos.z + size.z > pb.m_position.z) {
                return false;
            }
        }
        return true;
    };

    for (const auto& gene : ind.m_chromosomes) {
        const auto& box = allBoxes[gene.m_boxIndex];
        Vector3 size = getOrientatedSize(box.m_size, gene.m_orientation);

        std::sort(epList.begin(), epList.end(), [](const Vector3& a, const Vector3& b) {
            if (a.y != b.y) return a.y < b.y;
            if (a.z != b.z) return a.z < b.z;
            return a.x < b.x;
        });

        for (size_t i = 0; i < epList.size(); ++i) {
            Vector3 pt = epList[i];

            if (isValid(pt, size)) {
                placedBoxes.emplace_back(size, pt, box.m_color);
                epList.erase(epList.begin() + i);

                Vector3 p1 = {pt.x + size.x, pt.y, pt.z};
                Vector3 p2 = {pt.x, pt.y + size.y, pt.z};
                Vector3 p3 = {pt.x, pt.y, pt.z + size.z};

                auto addEP = [&](Vector3 p) {
                    if (p.x >= containerSize.x || p.y >= containerSize.y || p.z >= containerSize.z)
                        return;
                    for (const auto& existing : epList) {
                        if (existing.x == p.x && existing.y == p.y && existing.z == p.z) return;
                    }
                    epList.push_back(p);
                };

                addEP(p1);
                addEP(p2);
                addEP(p3);

                epList.erase(std::remove_if(epList.begin(), epList.end(),
                                            [&](const Vector3& ep) {
                                                return (ep.x >= pt.x && ep.x < pt.x + size.x &&
                                                        ep.y >= pt.y && ep.y < pt.y + size.y &&
                                                        ep.z >= pt.z && ep.z < pt.z + size.z);
                                            }),
                             epList.end());

                break;
            }
        }
    }

    return placedBoxes;
}

std::vector<PlacedBox> ISolver::decodeGreedy(const Individual& ind) const {
    std::vector<PlacedBox> placedBoxes;
    placedBoxes.reserve(ind.m_chromosomes.size());

    const auto& containerSize = m_model->getContainerSize();
    const auto& allBoxes = m_model->getBoxes();

    float cx = 0.0f, cy = 0.0f, cz = 0.0f;
    float maxH = 0.0f, maxD = 0.0f;

    for (const auto& gene : ind.m_chromosomes) {
        const auto& box = allBoxes[gene.m_boxIndex];
        Vector3 size = getOrientatedSize(box.m_size, gene.m_orientation);

        if (cx + size.x > containerSize.x) {
            cx = 0.0f;
            cz += maxD;
            maxD = 0.0f;
        }

        if (cz + size.z > containerSize.z) {
            cx = 0.0f;
            cz = 0.0f;
            cy += maxH;
            maxH = 0.0f;
        }

        if (cy + size.y > containerSize.y) {
            continue;
        }

        placedBoxes.emplace_back(size, Vector3{cx, cy, cz}, box.m_color);

        cx += size.x;
        maxH = std::max(maxH, size.y);
        maxD = std::max(maxD, size.z);
    }

    return placedBoxes;
}
