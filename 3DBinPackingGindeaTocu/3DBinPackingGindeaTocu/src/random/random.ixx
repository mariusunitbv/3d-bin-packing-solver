module;
#include <pch.h>

export module random;

export class Random {
   public:
    static Random& get();

    static int getInt(int min, int max);
    static float getFloat(float min, float max);

    static std::mt19937& rng();

   private:
    std::mt19937 m_rng{std::random_device{}()};
};
