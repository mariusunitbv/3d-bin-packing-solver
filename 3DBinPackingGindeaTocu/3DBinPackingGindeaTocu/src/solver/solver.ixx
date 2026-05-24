module;
#include <pch.h>

export module solver;

export import model;

import individual;

export enum SolverType_t {
    SOLVER_BLB = 0,
    SOLVER_LAYERBASED,
    SOLVER_EXTREME_POINTS,
    SOLVER_GREEDY,
    SOLVER_GENETIC,
    SOLVER_HILLCLIMBING,

    SOLVER_MAX
};

export constexpr const char* getSolverName(SolverType_t solver) {
    switch (solver) {
        case SOLVER_BLB:
            return "Bottom-Left-Back";
        case SOLVER_LAYERBASED:
            return "Layer-Based";
        case SOLVER_EXTREME_POINTS:
            return "Extreme Points";
        case SOLVER_GREEDY:
            return "Greedy (Volume-Based)";
        case SOLVER_GENETIC:
            return "Genetic Algorithm";
        case SOLVER_HILLCLIMBING:
            return "Hill Climbing";
        default:
            return "Unknown";
    }
}

export struct SolverStats {
    SolverType_t m_solverType;
    float m_volumeUtilization;
    int m_boxesPlaced;
    int m_totalBoxes;

    float m_timeTaken;

    // Percentage X%
    int getBoxesUtilization() const {
        return m_totalBoxes > 0 ? (m_boxesPlaced * 100) / m_totalBoxes : 0;
    }
};

export class ISolver {
   public:
    static std::unique_ptr<ISolver> create(SolverType_t type);

    virtual ~ISolver() = default;

    void setModel(ProblemModel* model);
    void setParameter(const std::string& key, const std::string& value);
    const std::vector<PlacedBox>& getSolution() const;
    const SolverStats& getStats() const;

    void setExistingSolution(const std::vector<PlacedBox>& solution);
    void setDecoderType(SolverType_t decoder);

    virtual void initializeParameters() {}

    void solve();

   protected:
    virtual void solveInternal() = 0;

    Individual createRandomIndividual();

    bool isValidPlacement(const PlacedBox& candidate) const;
    Orientation getRandomOrientation(const Box& box) const;

    std::vector<PlacedBox> decodeIndividual(const Individual& ind) const;
    double evaluateFitness(const Individual& ind) const;
    void mutate(Individual& ind);

    ISolver() = default;

    ProblemModel* m_model = nullptr;

    std::unordered_map<std::string, std::string> m_parameters;
    std::vector<int> m_unplacedBoxes;
    std::vector<PlacedBox> m_solution;
    SolverStats m_stats{};

    SolverType_t m_decoderType{SOLVER_BLB};

   private:
    std::vector<PlacedBox> decodeBLB(const Individual& ind) const;
    std::vector<PlacedBox> decodeLayerBased(const Individual& ind) const;
    std::vector<PlacedBox> decodeExtremePoints(const Individual& ind) const;
    std::vector<PlacedBox> decodeGreedy(const Individual& ind) const;
};
