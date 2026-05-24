module;
#include <pch.h>

module hillclimbing_solver;

HillClimbingSolver::HillClimbingSolver() { m_stats.m_solverType = SOLVER_HILLCLIMBING; }

void HillClimbingSolver::initializeParameters() {
    m_maxIterations = std::stoi(m_parameters["maxIterations"]);
}

void HillClimbingSolver::solveInternal() {
    Individual current = createRandomIndividual();
    m_bestIndividual = current;

    for (int i = 0; i < m_maxIterations; ++i) {
        Individual neighbor = current;
        mutate(neighbor);

        neighbor.m_fitness = evaluateFitness(neighbor);

        if (neighbor.m_fitness > current.m_fitness) {
            current = neighbor;

            if (neighbor.m_fitness > m_bestIndividual.m_fitness) {
                m_bestIndividual = neighbor;
            }
        }

        std::cout << "Iteration " << i + 1 << "/" << m_maxIterations
                  << " - Current fitness: " << current.m_fitness
                  << " - Best fitness: " << m_bestIndividual.m_fitness << '\n';
    }

    m_solution = decodeIndividual(m_bestIndividual);
}
