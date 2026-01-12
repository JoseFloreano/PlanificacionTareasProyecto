#include "mutations.h"

#include <algorithm>
#include <numeric>

// =======================
// INTER-CHROMOSOME
// =======================
void mutationInterChromosome(Individual& individual,
                             std::mt19937& rng,
                             float mutationRate,
                             std::uniform_real_distribution<double>& dist)
{
    if (dist(rng) < mutationRate) {
        std::uniform_int_distribution<int> distInt(0, individual.getNumChromosomes() - 1);

        int a = distInt(rng);
        int b;
        do {
            b = distInt(rng);
        } while (b == a);

        std::swap(individual.chromosomes[a].genes,
                  individual.chromosomes[b].genes);
    }
}

// =======================
// RECIPROCAL EXCHANGE
// =======================
void mutationReciprocalExchange(Individual& individual,
                                std::mt19937& rng,
                                float mutationRate,
                                std::uniform_real_distribution<double>& dist)
{
    if (dist(rng) < mutationRate) {
        std::uniform_int_distribution<int> distK(1, 3);
        int n = individual.chromosomes[0].genes.size();

        for (int c = 0; c < individual.getNumChromosomes(); c++) {
            int k = distK(rng);

            std::vector<int> indexes(n);
            std::iota(indexes.begin(), indexes.end(), 0);
            std::shuffle(indexes.begin(), indexes.end(), rng);

            for (int pairCount = 0; pairCount < k; pairCount++) {
                int i = indexes[2 * pairCount];
                int j = indexes[2 * pairCount + 1];

                std::swap(individual.chromosomes[c].genes[i],
                          individual.chromosomes[c].genes[j]);
            }
        }
    }
}

// =======================
// SHIFT MUTATION
// =======================
void mutationShift(Individual& individual,
                   std::mt19937& rng,
                   float mutationRate,
                   std::uniform_real_distribution<double>& dist)
{
    if (dist(rng) < mutationRate) {
        std::uniform_int_distribution<int> distWindow(3, 5);
        int windowSize = distWindow(rng);

        int n = individual.chromosomes[0].genes.size();

        for (int c = 0; c < individual.getNumChromosomes(); c++) {
            std::uniform_int_distribution<int> distStart(0, n - windowSize);
            int startIdx = distStart(rng);

            int last = individual.chromosomes[c].genes[startIdx + windowSize - 1];

            for (int i = startIdx + windowSize - 1; i > startIdx; i--) {
                individual.chromosomes[c].genes[i] =
                    individual.chromosomes[c].genes[i - 1];
            }

            individual.chromosomes[c].genes[startIdx] = last;
        }
    }
}
