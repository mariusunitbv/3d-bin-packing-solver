module;
#include <pch.h>

module genetic_solver;

import random;
import benchmark_timer;

GeneticSolver::GeneticSolver() { m_stats.m_solverType = SOLVER_GENETIC; }

void GeneticSolver::initializeParameters() {
    m_populationSize = std::stoi(m_parameters["population"]);
    m_generations = std::stoi(m_parameters["generations"]);
    m_mutationRate = std::stof(m_parameters["mutation"]);
    m_tournamentSize = std::stoi(m_parameters["tournament"]);
    m_optimalFitness = std::stoi(m_parameters["optimal"]);
}

void GeneticSolver::solveInternal() {
    createInitialPopulation();

    for (int generation = 0; generation < m_generations; ++generation) {
        m_population = selectNewPopulation();
        if (m_population.size() % 2 != 0) {
            m_population.pop_back();
        }

        m_population = crossoverAndMutate();

#ifdef __EMSCRIPTEN__
        for (size_t i = 0; i < m_population.size(); ++i) {
            m_population[i].m_fitness = evaluateFitness(m_population[i]);
        }
#else
        std::for_each(std::execution::par, m_population.begin(), m_population.end(),
                      [this](Individual& ind) { ind.m_fitness = evaluateFitness(ind); });
#endif

        auto& bestOfGeneration = getBestIndividual();
        if (bestOfGeneration.m_fitness > m_bestIndividual.m_fitness) {
            m_bestIndividual = bestOfGeneration;
        }

        std::cout << "Generation " << generation + 1 << "/" << m_generations
                  << " - Best fitness: " << bestOfGeneration.m_fitness << '\n';

        if (bestOfGeneration.m_fitness >= m_optimalFitness) {
            std::cout << "Perfect solution found in generation " << generation + 1 << "!\n";
            break;
        }
    }

    m_solution = decodeIndividual(m_bestIndividual);
}

void GeneticSolver::createInitialPopulation() {
    BenchmarkTimer timer("Creating initial population");

    m_population.resize(m_populationSize);

#ifdef __EMSCRIPTEN__
    for (size_t i = 0; i < m_population.size(); ++i) {
        m_population[i] = createRandomIndividual();
    }
#else
    std::for_each(std::execution::par, m_population.begin(), m_population.end(),
                  [this](Individual& ind) { ind = createRandomIndividual(); });
#endif

    for (size_t i = 0; i < m_population.size(); ++i) {
        std::cout << "Created individual " << i + 1 << "/" << m_populationSize
                  << " with fitness: " << m_population[i].m_fitness << '\n';
    }
}

std::vector<Individual> GeneticSolver::selectNewPopulation() {
    std::vector<Individual> newPopulation;
    newPopulation.reserve(m_populationSize);

    for (int i = 0; i < m_populationSize; ++i) {
        int bestIndex = Random::getInt(0, m_populationSize - 1);
        for (int j = 1; j < m_tournamentSize; ++j) {
            int idx = Random::getInt(0, m_populationSize - 1);
            if (m_population[idx].m_fitness > m_population[bestIndex].m_fitness) {
                bestIndex = idx;
            }
        }

        newPopulation.push_back(m_population[bestIndex]);
    }

    return newPopulation;
}

std::vector<Individual> GeneticSolver::crossoverAndMutate() {
    std::vector<Individual> newPopulation;

    for (int i = 0; i < m_population.size() - 1; i += 2) {
        const Individual& parent1 = m_population[i];
        const Individual& parent2 = m_population[i + 1];

        Individual child1 = crossover(parent1, parent2);
        Individual child2 = crossover(parent2, parent1);

        if (Random::getFloat(0.0f, 1.0f) < m_mutationRate) {
            mutate(child1);
        }

        if (Random::getFloat(0.0f, 1.0f) < m_mutationRate) {
            mutate(child2);
        }

        newPopulation.push_back(child1);
        newPopulation.push_back(child2);
    }

    return newPopulation;
}

Individual& GeneticSolver::getBestIndividual() {
    Individual* best = &m_population[0];

    for (auto& ind : m_population) {
        if (ind.m_fitness > best->m_fitness) {
            best = &ind;
        }
    }

    return *best;
}

Individual GeneticSolver::crossover(const Individual& parent1, const Individual& parent2) {
    Individual child;
    size_t size = parent1.m_chromosomes.size();
    child.m_chromosomes.reserve(size);

    const auto crossoverPoint = Random::getInt(1, (int)size - 2);
    std::vector<bool> used(m_model->getBoxes().size(), false);
    for (size_t i = 0; i < static_cast<size_t>(crossoverPoint); ++i) {
        child.m_chromosomes.push_back(parent1.m_chromosomes[i]);
        used[parent1.m_chromosomes[i].m_boxIndex] = true;
    }

    for (size_t i = 0; i < size; ++i) {
        uint32_t boxId = parent2.m_chromosomes[i].m_boxIndex;
        if (!used[boxId]) {
            child.m_chromosomes.push_back(parent2.m_chromosomes[i]);
            used[boxId] = true;
        }
    }

    return child;
}
