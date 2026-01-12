#include "EvolutionaryAlgorithm.h"
#include <algorithm>
#include <limits>
#include <cmath>
#include "mutations.h"
#include <QDebug>

// ================= Constructor =================

EvolutionaryAlgorithm::EvolutionaryAlgorithm(const ScenarioData& scenario,
                                             const QVector<QString>& policyNames,
                                             int populationSize,
                                             float crossoverRate,
                                             const QVector<float>& mutationRates)
    : scenario(scenario),
    policyNames(policyNames),
    populationSize(populationSize),
    crossoverRate(crossoverRate),
    rng(std::random_device{}()),
    dist(0.0, 1.0) {
    if (mutationRates.size() != MutationCount) {
        throw std::runtime_error("Mutation rates size mismatch");
    }
    this->mutationRates = mutationRates;
}

// ================= Inicialización =================

void EvolutionaryAlgorithm::initialize() {
    population.clear();
    population.reserve(populationSize);

    for (int i = 0; i < populationSize; ++i) {
        Individual ind(policyNames);
        ind.initializeRandom(scenario.totalOperations, 1, scenario.numMachines,
                             *reinterpret_cast<QRandomGenerator*>(&rng));
        evaluateIndividual(ind);
        population.append(ind);
    }
    fastNonDominatedSort(population);

    f1_max = population[0].chromosomes[0].f1;
    f2_max = population[0].chromosomes[0].f2;

    for (auto& ind : population) {
        for (auto& chrom : ind.chromosomes) {
            if (chrom.f1 > f1_max) f1_max = chrom.f1;
            if (chrom.f2 > f2_max) f2_max = chrom.f2;
        }
    }

    f1_max += 50;
    f2_max += 50;
    qDebug() << "Inicializado";
    QVector<double> hyperTemp;
    for (int i=0; i<population[0].getNumChromosomes(); i++){
        double hv = calculateHyperVolume(i, f1_max, f2_max);
        hyperTemp.append(hv);
    }
    qDebug() << "Inicializado";
    hypervolumes.append(hyperTemp);
}

// ================= Generación =================

void EvolutionaryAlgorithm::runGeneration() {
    QVector<Individual> parents = selectParents(population);
    QVector<Individual> offspring = uniformCrossoverPopulation(parents);

    evaluatePopulation(offspring);

    QVector<Individual> combined = population;
    combined += offspring;

    applyMutations(combined);
    fastNonDominatedSort(combined);

    population.clear();
    for (int i = 0; i < populationSize; ++i) {
        population.append(tournamentSelection(combined));
    }

    evaluatePopulation(population);
    fastNonDominatedSort(population);

    QVector<double> hyperTemp;
    for (int i=0; i<population[0].getNumChromosomes(); i++){
        double hv = calculateHyperVolume(i, f1_max, f2_max);
        hyperTemp.append(hv);
    }

    hypervolumes.append(hyperTemp);
}

// ================= Evaluación =================

void EvolutionaryAlgorithm::evaluateIndividual(Individual& individual) {
    for (Chromosome& c : individual.chromosomes) {
        evaluateChromosome(c);
    }
}

void EvolutionaryAlgorithm::evaluatePopulation(QVector<Individual>& pop) {
    for (Individual& ind : pop) {
        evaluateIndividual(ind);
    }
}

double EvolutionaryAlgorithm::calculateHyperVolume(int chromosomeIndex,
                                                   double refPointF1,
                                                   double refPointF2) const
{
    QVector<QPair<double, double>> points;

    // Recolectar puntos del frente Pareto (domLevel == 1)
    for (const Individual &ind : population) {
        if (chromosomeIndex >= ind.chromosomes.size())
            continue;

        const Chromosome &c = ind.chromosomes[chromosomeIndex];

        if (c.domLevel == 1) {
            points.append(qMakePair(c.f1, c.f2));
        }
    }

    // Ordenar por f1 ascendente (igual que std::sort en pair)
    std::sort(points.begin(), points.end(),
              [](const QPair<double, double> &a,
                 const QPair<double, double> &b) {
                  return a.first < b.first;
              });

    double hypervolume = 0.0;
    double prevF1 = refPointF1;

    // Cálculo del hipervolumen
    for (const auto &point : points) {
        double f1 = point.first;
        double f2 = point.second;

        if (f1 < refPointF1 && f2 < refPointF2) {
            hypervolume += (prevF1 - f1) * (refPointF2 - f2);
            prevF1 = f1;
        }
    }

    return hypervolume;
}

// ================= NSGA-II =================

void EvolutionaryAlgorithm::fastNonDominatedSort(QVector<Individual>& pop) {
    for (int c = 0; c < population[0].getNumChromosomes();c++){
        int rank = 0;
        QVector<Individual*> dominatedIndividuals;
        QVector<Individual*> dominatedIndividualsTemp;
        for (auto& ind : population) {
            dominatedIndividuals.push_back(&ind);
        }
        while (!dominatedIndividuals.empty()) {
            rank++;
            for (size_t i = 0; i < dominatedIndividuals.size(); ++i) {
                bool isDominated = false;
                for (size_t j = 0; j < dominatedIndividuals.size(); ++j) {
                    if (i == j) continue;
                    auto& A = dominatedIndividuals[j]; // posible dominante
                    auto& B = dominatedIndividuals[i]; // posible dominado

                    bool betterOrEqualInAll =
                        A->chromosomes[c].f1 <= B->chromosomes[c].f1 &&
                        A->chromosomes[c].f2 <= B->chromosomes[c].f2;

                    bool strictlyBetterInAtLeastOne =
                        A->chromosomes[c].f1 < B->chromosomes[c].f1 ||
                        A->chromosomes[c].f2 < B->chromosomes[c].f2;

                    if (betterOrEqualInAll && strictlyBetterInAtLeastOne) {
                        isDominated = true;
                        break;
                    }
                }

                if (isDominated)
                    dominatedIndividualsTemp.push_back(dominatedIndividuals[i]);
                else
                    dominatedIndividuals[i]->chromosomes[c].domLevel = rank;
            }
            dominatedIndividuals = dominatedIndividualsTemp;
            dominatedIndividualsTemp.clear();
        }
        for (int r = 1; r <= rank; r++) {
            QVector<Individual*> front;
            for (auto& ind : population) {
                if (ind.chromosomes[c].domLevel == r) {
                    front.push_back(&ind);
                }
            }
            calculateCrowdingDistance(front, c);
        }
    }
}

void EvolutionaryAlgorithm::calculateCrowdingDistance(QVector<Individual*>& front, int chromosomeIndex) {
    int size = front.size();
    if (size == 0) return;

    for (auto& ind : front) {
        ind->chromosomes[chromosomeIndex].crowdingDistance = 0;
    }

    auto compareF1 = [chromosomeIndex](Individual* a, Individual* b) {
        return a->chromosomes[chromosomeIndex].f1 < b->chromosomes[chromosomeIndex].f1;
    };
    auto compareF2 = [chromosomeIndex](Individual* a, Individual* b) {
        return a->chromosomes[chromosomeIndex].f2 < b->chromosomes[chromosomeIndex].f2;
    };

    std::sort(front.begin(), front.end(), compareF1);
    front[0]->chromosomes[chromosomeIndex].crowdingDistance = std::numeric_limits<double>::infinity();
    front[size - 1]->chromosomes[chromosomeIndex].crowdingDistance = std::numeric_limits<double>::infinity();

    double f1Min = front[0]->chromosomes[chromosomeIndex].f1;
    double f1Max = front[size - 1]->chromosomes[chromosomeIndex].f1;

    for (int i = 1; i < size - 1; i++) {
        if (f1Max - f1Min == 0) continue;
        front[i]->chromosomes[chromosomeIndex].crowdingDistance +=
            (front[i + 1]->chromosomes[chromosomeIndex].f1 - front[i - 1]->chromosomes[chromosomeIndex].f1) / (f1Max - f1Min);
    }

    std::sort(front.begin(), front.end(), compareF2);
    front[0]->chromosomes[chromosomeIndex].crowdingDistance = std::numeric_limits<double>::infinity();
    front[size - 1]->chromosomes[chromosomeIndex].crowdingDistance = std::numeric_limits<double>::infinity();

    double f2Min = front[0]->chromosomes[chromosomeIndex].f2;
    double f2Max = front[size - 1]->chromosomes[chromosomeIndex].f2;

    for (int i = 1; i < size - 1; i++) {
        if (f2Max - f2Min == 0) continue;
        front[i]->chromosomes[chromosomeIndex].crowdingDistance +=
            (front[i + 1]->chromosomes[chromosomeIndex].f2 - front[i - 1]->chromosomes[chromosomeIndex].f2) / (f2Max - f2Min);
    }
}

// ================= Selección =================

Individual EvolutionaryAlgorithm::tournamentSelection(
    const QVector<Individual>& pop
    )
{
    std::uniform_int_distribution<int> dist(0, pop.size() - 1);

    int index1 = dist(rng);
    int index2 = dist(rng);

    const Individual& A = pop[index1];
    const Individual& B = pop[index2];

    Individual superIndividual(policyNames);

    // Asegurar estructura
    superIndividual.chromosomes.resize(A.getNumChromosomes());

    for (int c = 0; c < A.getNumChromosomes(); c++) {

        const Chromosome* winner = nullptr;

        if (A.chromosomes[c].domLevel < B.chromosomes[c].domLevel) {
            winner = &A.chromosomes[c];
        }
        else if (B.chromosomes[c].domLevel < A.chromosomes[c].domLevel) {
            winner = &B.chromosomes[c];
        }
        else {
            winner = (A.chromosomes[c].crowdingDistance >
                      B.chromosomes[c].crowdingDistance)
                         ? &A.chromosomes[c]
                         : &B.chromosomes[c];
        }

        superIndividual.chromosomes[c].genes = winner->genes;
    }

    return superIndividual;
}


QVector<Individual> EvolutionaryAlgorithm::selectParents(
    const QVector<Individual>& pop
    )
{
    QVector<Individual> parents;
    parents.reserve(pop.size());

    for (int i = 0; i < pop.size(); i++) {
        parents.push_back(tournamentSelection(pop));
    }

    return parents;
}


// ================= Cruza =================

QVector<Individual>
EvolutionaryAlgorithm::uniformCrossoverPopulation(const QVector<Individual>& parents)
{
    if (parents.isEmpty()) {
        return {};
    }

    QVector<Individual> offspring;
    offspring.reserve(parents.size());

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < parents.size(); i += 2) {

        const Individual& parent1 = parents[i];
        const Individual& parent2 = parents[(i + 1) % parents.size()];

        Individual offspring1(policyNames);
        Individual offspring2(policyNames);

        // ===== UNIFORM CROSSOVER =====
        if (dist(rng) < crossoverRate) {

            int geneCount = parent1.chromosomes[0].genes.size();
            int numChromosomes = parent1.getNumChromosomes();

            for (int g = 0; g < geneCount; g++) {

                bool fromParent1 = (dist(rng) < 0.5);

                for (int c = 0; c < numChromosomes; c++) {
                    if (fromParent1) {
                        offspring1.chromosomes[c].genes.push_back(
                            parent1.chromosomes[c].genes[g]);
                        offspring2.chromosomes[c].genes.push_back(
                            parent2.chromosomes[c].genes[g]);
                    } else {
                        offspring1.chromosomes[c].genes.push_back(
                            parent2.chromosomes[c].genes[g]);
                        offspring2.chromosomes[c].genes.push_back(
                            parent1.chromosomes[c].genes[g]);
                    }
                }
            }

        } else {
            // Sin cruce
            offspring1 = parent1;
            offspring2 = parent2;
        }

        offspring.push_back(offspring1);
        offspring.push_back(offspring2);
    }

    return offspring;
}

// ================= Mutación =================

void EvolutionaryAlgorithm::applyMutations(QVector<Individual>& population)
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (Individual& individual : population) {

        mutationInterChromosome(
            individual, rng,
            mutationRates[InterChromosome], dist
            );

        mutationReciprocalExchange(
            individual, rng,
            mutationRates[ReciprocalExchange], dist
            );

        mutationShift(
            individual, rng,
            mutationRates[Shift], dist
            );
    }
}

// ================= Scheduling =================

double EvolutionaryAlgorithm::calculateStartTime(double m, double j) const {
    return std::max(m, j);
}

OperationSchedule EvolutionaryAlgorithm::scheduleOperation(
    int opId,
    int jobId,
    int machineId,
    QVector<MachineState>& machines,
    QVector<JobState>& jobStates
    ) const
{
    OperationSchedule schedule;

    // =============================
    // Información básica
    // =============================
    schedule.operationId = opId;
    schedule.jobId = jobId;
    schedule.machineId = machineId;

    // =============================
    // Datos del escenario
    // =============================
    schedule.processingTime =
        scenario.processingTime[opId][machineId];

    schedule.energyCost =
        scenario.energyCost[opId][machineId];

    // =============================
    // Cálculo de inicio
    // =============================
    schedule.startTime = std::max(
        machines[machineId].currentTime,
        jobStates[jobId].lastOperationEndTime
        );

    // =============================
    // Finalización
    // =============================
    schedule.endTime =
        schedule.startTime + schedule.processingTime;

    // =============================
    // Actualizar máquina
    // =============================
    machines[machineId].currentTime = schedule.endTime;
    machines[machineId].totalEnergy += schedule.energyCost;
    machines[machineId].isActive = true;

    // =============================
    // Actualizar trabajo
    // =============================
    jobStates[jobId].lastOperationEndTime = schedule.endTime;
    jobStates[jobId].nextOperationIndex++;

    return schedule;
}

QVector<OperationSchedule>
EvolutionaryAlgorithm::evaluateChromosome(Chromosome& chromosome) {
    QVector<MachineState> machines(scenario.numMachines);
    QVector<JobState> jobStates(scenario.numJobs);
    QVector<OperationSchedule> schedule;

    auto& mapping = scenario.chromosomeMapping[chromosome.policyName];

    for (int i = 0; i < chromosome.genes.size(); ++i) {
        int opId = mapping[i].second.id;
        int jobId = mapping[i].first.id;
        int machineId = chromosome.genes[i] - 1;

        schedule.append(scheduleOperation(opId, jobId, machineId,
                                          machines, jobStates));
    }

    double makespan = 0, energy = 0;
    for (const auto& m : machines) {
        if (m.isActive) {
            makespan = std::max(makespan, m.currentTime);
            energy += m.totalEnergy;
        }
    }

    chromosome.f1 = makespan;
    chromosome.f2 = energy;

    return schedule;
}

// ================= Setters =================

void EvolutionaryAlgorithm::setMutationRates(const QVector<float>& rates)
{
    if (rates.size() != MutationCount) {
        throw std::runtime_error("Mutation rates size mismatch");
    }
    mutationRates = rates;
}


void EvolutionaryAlgorithm::setCrossoverRate(float rate)
{
    crossoverRate = rate;
}

// ================= Getters =================

const QVector<Individual>& EvolutionaryAlgorithm::getPopulation() const {
    return population;
}

const QVector<QVector<double>>& EvolutionaryAlgorithm::getHypervolumes() const {
    return hypervolumes;
}

Individual EvolutionaryAlgorithm::getBestMakespan() const
{
    double bestMakespan = std::numeric_limits<double>::max();
    Individual bestIndividual = population[0];

    for (const Individual &ind : population) {
        int numChrom = ind.getNumChromosomes();

        for (int c = 0; c < numChrom; ++c) {
            const Chromosome &chrom = ind.chromosomes[c];

            if (chrom.f1 < bestMakespan) {
                bestMakespan = chrom.f1;
                bestIndividual = ind;
            }
        }
    }

    return bestIndividual;
}

Individual EvolutionaryAlgorithm::getBestEnergy() const
{
    double bestEnergy = std::numeric_limits<double>::max();
    Individual bestIndividual = population[0];

    for (const Individual &ind : population) {
        int numChrom = ind.getNumChromosomes();

        for (int c = 0; c < numChrom; ++c) {
            const Chromosome &chrom = ind.chromosomes[c];

            if (chrom.f2 < bestEnergy) {
                bestEnergy = chrom.f2;
                bestIndividual = ind;
            }
        }
    }

    return bestIndividual;
}

Individual EvolutionaryAlgorithm::getKneePoint() const
{
    double refF1 = std::numeric_limits<double>::max();
    double refF2 = std::numeric_limits<double>::max();

    // -------------------------
    // 1) Encontrar punto ideal
    // -------------------------
    for (const Individual &ind : population) {
        int numChrom = ind.getNumChromosomes();

        for (int c = 0; c < numChrom; ++c) {
            const Chromosome &chrom = ind.chromosomes[c];

            if (chrom.domLevel == 1) {
                refF1 = std::min(refF1, chrom.f1);
                refF2 = std::min(refF2, chrom.f2);
            }
        }
    }

    // -------------------------
    // 2) Buscar knee point
    // -------------------------
    double minDistance = std::numeric_limits<double>::max();
    Individual kneePoint = population[0];

    for (const Individual &ind : population) {
        int numChrom = ind.getNumChromosomes();

        for (int c = 0; c < numChrom; ++c) {
            const Chromosome &chrom = ind.chromosomes[c];

            if (chrom.domLevel == 1) {
                double distance = std::sqrt(
                    std::pow(chrom.f1 - refF1, 2) +
                    std::pow(chrom.f2 - refF2, 2)
                    );

                if (distance < minDistance) {
                    minDistance = distance;
                    kneePoint = ind;
                }
            }
        }
    }

    return kneePoint;
}



