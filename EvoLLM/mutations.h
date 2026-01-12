#ifndef MUTATIONS_H
#define MUTATIONS_H

#include <random>
#include "individual.h"

// Mutaciones individuales
void mutationInterChromosome(Individual& individual,
                             std::mt19937& rng,
                             float mutationRate,
                             std::uniform_real_distribution<double>& dist);

void mutationReciprocalExchange(Individual& individual,
                                std::mt19937& rng,
                                float mutationRate,
                                std::uniform_real_distribution<double>& dist);

void mutationShift(Individual& individual,
                   std::mt19937& rng,
                   float mutationRate,
                   std::uniform_real_distribution<double>& dist);

#endif // MUTATIONS_H
