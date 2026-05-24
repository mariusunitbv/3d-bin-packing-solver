module;
#include <pch.h>

export module genetic_solver;

export import solver;

import individual;

export class GeneticSolver : public ISolver {
   public:
    GeneticSolver();

   private:
    void initializeParameters() override;
    void solveInternal() override;

    void createInitialPopulation();

    std::vector<Individual> selectNewPopulation();
    std::vector<Individual> crossoverAndMutate();

    Individual& getBestIndividual();

    Individual crossover(const Individual& parent1, const Individual& parent2);

    int m_populationSize;
    int m_generations;
    float m_mutationRate;
    int m_tournamentSize;
    int m_optimalFitness;

    std::vector<Individual> m_population;
    Individual m_bestIndividual;
};
