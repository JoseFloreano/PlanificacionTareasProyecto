#ifndef SCENARIO_DATA_H
#define SCENARIO_DATA_H

#include <QVector>
#include <QMap>
#include <QString>
#include "ScenarioTypes.h"      // Contiene definiciones como struct Job, struct Operation, etc.

/**
 * @brief Estructura que contiene TODA la información estática del problema de scheduling
 * 
 * Representa una instancia del problema (job shop, flexible job shop, etc.)
 * con tiempos de procesamiento y costos energéticos para cada operación en cada máquina.
 * 
 * Esta estructura es de solo lectura una vez cargada y se comparte entre todos los
 * individuos y evaluaciones del algoritmo evolutivo.
 */
struct ScenarioData {
    int numOperations;          // Número de operaciones únicas (puede no ser igual a totalOperations)
                                // Ejemplo: si hay 3 jobs con 4, 3 y 5 operaciones → numOperations = 12

    int totalOperations;        // Suma total de todas las operaciones de todos los jobs
                                // (normalmente igual a numOperations en la mayoría de casos)

    int numMachines;            // Número total de máquinas disponibles
    int numJobs;                // Número total de trabajos (jobs) a programar

    /**
     * Matriz [job][operation] → tiempo de procesamiento en cada máquina posible
     * processingTime[jobIdx][opIdx][machineIdx] no se usa directamente,
     * normalmente se accede mediante jobs[jobIdx].operations[opIdx].processingTimes[machine]
     */
    QVector<QVector<double>> processingTime;

    /**
     * Matriz [job][operation] → costo energético en cada máquina posible
     * Similar a processingTime, pero para consumo de energía
     */
    QVector<QVector<double>> energyCost;

    /**
     * Lista completa de todos los trabajos
     * Cada Job contiene:
     * - Lista ordenada de operaciones
     * - Para cada operación: máquinas posibles + tiempos + consumos energéticos
     */
    QVector<Job> jobs;

    /**
     * Mapeo utilizado en algunas políticas/codificaciones
     * Asocia un nombre de política → secuencia de asignaciones (Job, Operation)
     * 
     * Se usa principalmente cuando la codificación del cromosoma es del tipo
     * "secuencia de operaciones" y se necesita mapear genes a operaciones concretas.
     * 
     * Ejemplo: "FIFO" → lista ordenada de todas las operaciones en cierto orden
     */
    QMap<QString, QVector<QPair<Job, Operation>>> chromosomeMapping;

    /**
     * Constructor por defecto
     * Inicializa todos los contadores en 0
     * Los vectores y mapas quedan vacíos automáticamente
     */
    ScenarioData()
        : numOperations(0),
          totalOperations(0),
          numMachines(0),
          numJobs(0)
    {}
};

#endif // SCENARIO_DATA_H