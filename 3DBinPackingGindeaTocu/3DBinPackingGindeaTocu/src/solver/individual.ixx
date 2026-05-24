module;
#include <pch.h>

export module individual;

export import box;

export struct Gene {
    uint32_t m_boxIndex;
    Orientation m_orientation;
};

export struct Individual {
    std::vector<Gene> m_chromosomes;
    double m_fitness = -1.;
};
