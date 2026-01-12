#ifndef EVOLUTIONARY_ALGORITHM_H
#define EVOLUTIONARY_ALGORITHM_H

#include <QVector>
#include <random>

#include "Individual.h"
#include "ScenarioData.h"
#include "ScheduleTypes.h"

enum MutationType {
    InterChromosome = 0,
    ReciprocalExchange,
    Shift,
    MutationCount
};

class EvolutionaryAlgorithm {
public:
    EvolutionaryAlgorithm(const ScenarioData& scenario,
                          const QVector<QString>& policyNames,
                          int populationSize,
                          float crossoverRate,
                          const QVector<float>& mutationRates);

    // Inicialización
    void initialize();

    // Ejecuta una generación completa
    void runGeneration();

    // Setters
    void setMutationRates(const QVector<float>& rates);
    void setCrossoverRate(float rate);

    // Getters
    const QVector<Individual>& getPopulation() const;
    const QVector<QVector<double>>& getHypervolumes() const;

    Individual getKneePoint() const;
    Individual getBestMakespan() const;
    Individual getBestEnergy() const;

    double calculateHyperVolume(int chromosomeIndex,
                                double refPointF1,
                                double refPointF2) const;

    QVector<OperationSchedule> evaluateChromosome(Chromosome& chromosome);

private:
    // === Estado ===
    ScenarioData scenario;
    QVector<QString> policyNames;
    QVector<Individual> population;
    QVector<QVector<double>> hypervolumes;
    double f1_max;
    double f2_max;

    int populationSize;

    // RNG
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

    // Parámetros GA
    float crossoverRate;
    QVector<float> mutationRates;

    // === Evaluación ===
    void evaluateIndividual(Individual& individual);
    void evaluatePopulation(QVector<Individual>& pop);

    // === NSGA-II ===
    void fastNonDominatedSort(QVector<Individual>& pop);
    void calculateCrowdingDistance(QVector<Individual*>& front, int chromosomeIndex);

    // === Selección ===
    Individual tournamentSelection(const QVector<Individual>& pop);
    QVector<Individual> selectParents(const QVector<Individual>& pop);

    // === Cruza ===
    QVector<Individual> uniformCrossoverPopulation(const QVector<Individual>& parents);

    // === Mutación ===
    void applyMutations(QVector<Individual>& population);

    // === Utilidades de scheduling ===
    double calculateStartTime(double machineAvailableTime,
                              double jobLastOperationTime) const;

    OperationSchedule scheduleOperation(int opId,
                                        int jobId,
                                        int machineId,
                                        QVector<MachineState>& machines,
                                        QVector<JobState>& jobStates) const;

};

#endif // EVOLUTIONARY_ALGORITHM_H
