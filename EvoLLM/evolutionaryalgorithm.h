#ifndef EVOLUTIONARY_ALGORITHM_H
#define EVOLUTIONARY_ALGORITHM_H

#include <QVector>
#include <random>                   // Para el generador Mersenne Twister (std::mt19937)
#include "Individual.h"             // Contiene la clase Individual (varios cromosomas)
#include "ScenarioData.h"           // Información del problema (jobs, máquinas, tiempos...)
#include "ScheduleTypes.h"          // Definiciones como OperationSchedule, MachineState, etc.

enum MutationType {
    InterChromosome = 0,        // Mutación entre diferentes cromosomas/políticas
    ReciprocalExchange,         // Intercambio recíproco dentro del mismo cromosoma
    Shift,                      // Desplazamiento (shift) de posiciones
    MutationCount               // Número total de tipos de mutación (usado como límite)
};

/**
 * @brief Implementación de un algoritmo evolutivo multiobjetivo (NSGA-II like)
 * para optimización de scheduling con dos objetivos: Makespan y Consumo Energético
 * 
 * Soporta múltiples políticas/codificaciones simultáneamente (cada Individual tiene
 * varios cromosomas, uno por política).
 */
class EvolutionaryAlgorithm {
public:
    /**
     * @brief Constructor principal
     * @param scenario Datos del problema de scheduling
     * @param policyNames Nombres de las diferentes políticas/codificaciones a evolucionar
     * @param populationSize Tamaño de la población
     * @param crossoverRate Probabilidad inicial de cruce (se puede adaptar)
     * @param mutationRates Probabilidades iniciales de cada tipo de mutación
     */
    EvolutionaryAlgorithm(const ScenarioData& scenario,
                         const QVector<QString>& policyNames,
                         int populationSize,
                         float crossoverRate,
                         const QVector<float>& mutationRates);

    // Inicialización
    /** Genera la población inicial con cromosomas aleatorios para cada política */
    void initialize();

    /** Ejecuta una generación completa: evaluación → ordenamiento → selección → cruce → mutación */
    void runGeneration();

    // Setters (útiles para adaptación dinámica de parámetros)
    void setMutationRates(const QVector<float>& rates);
    void setCrossoverRate(float rate);

    // Getters principales
    const QVector<Individual>& getPopulation() const;               // Población actual
    const QVector<QVector<double>>& getHypervolumes() const;        // Histórico de hipervolúmenes
    Individual getKneePoint() const;                                // Punto de rodilla (trade-off equilibrado)
    Individual getBestMakespan() const;                             // Mejor solución en Makespan
    Individual getBestEnergy() const;                               // Mejor solución en consumo energético

    /**
     * @brief Calcula el hipervolumen para un cromosoma específico (una política)
     * respecto a un punto de referencia
     */
    double calculateHyperVolume(int chromosomeIndex,
                               double refPointF1,
                               double refPointF2) const;

    /**
     * @brief Decodifica y evalúa un cromosoma generando su programación completa
     * @return Secuencia de operaciones programadas (scheduling real)
     */
    QVector<OperationSchedule> evaluateChromosome(Chromosome& chromosome);

private:
    // === Estado ===
    ScenarioData scenario;                          // Datos del problema (fijo)
    QVector<QString> policyNames;                   // Nombres de las políticas en uso
    QVector<Individual> population;                 // Población actual
    QVector<QVector<double>> hypervolumes;          // Registro histórico de hipervolúmenes por generación
    double f1_max, f2_max;                          // Peores valores observados (para normalización)
    int populationSize;

    // RNG (generador aleatorio de alta calidad)
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;    // Distribución [0,1) usada en decisiones probabilísticas

    // Parámetros GA (pueden cambiar durante la ejecución si hay adaptación)
    float crossoverRate;
    QVector<float> mutationRates;                   // Una tasa por cada tipo de mutación

    // === Evaluación ===
    void evaluateIndividual(Individual& individual);           // Evalúa todos los cromosomas de un individuo
    void evaluatePopulation(QVector<Individual>& pop);         // Evalúa toda una población

    // === NSGA-II ===
    void fastNonDominatedSort(QVector<Individual>& pop);       // Ordenamiento rápido por dominancia
    void calculateCrowdingDistance(QVector<Individual*>& front, int chromosomeIndex);
                                                                    // Diversidad dentro de cada frente

    // === Selección ===
    Individual tournamentSelection(const QVector<Individual>& pop);   // Selección por torneo binario
    QVector<Individual> selectParents(const QVector<Individual>& pop); // Selecciona padres para cruce

    // === Cruza ===
    QVector<Individual> uniformCrossoverPopulation(const QVector<Individual>& parents);
                                                                    // Cruza uniforme entre cromosomas

    // === Mutación ===
    void applyMutations(QVector<Individual>& population);      // Aplica mutaciones según tasas actuales

    // === Utilidades de scheduling ===
    double calculateStartTime(double machineAvailableTime,
                             double jobLastOperationTime) const;   // Lógica de no solapamiento

    OperationSchedule scheduleOperation(int opId,
                                       int jobId,
                                       int machineId,
                                       QVector<MachineState>& machines,
                                       QVector<JobState>& jobStates) const;
                                                                    // Programa una operación concreta
};

#endif // EVOLUTIONARY_ALGORITHM_H