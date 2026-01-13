#ifndef SCENARIO_TYPES_H
#define SCENARIO_TYPES_H

#include <QVector>
#include <QString>
#include <QMap>

/**
 * @file ScenarioTypes.h
 * @brief Definiciones de las estructuras básicas que representan los elementos
 * del problema de scheduling (operaciones y trabajos)
 * 
 * Estas estructuras son usadas en ScenarioData y en todo el proceso de
 * evaluación/decodificación de soluciones.
 */

/**
 * @brief Representa una operación individual dentro de un trabajo
 * 
 * En la mayoría de los casos, una operación es una tarea que debe realizarse
 * en una (o varias) máquinas específicas, con un tiempo de procesamiento y
 * costo energético asociado por máquina.
 * 
 * Actualmente solo guarda el identificador, ya que los tiempos y costos se
 * almacenan en las matrices globales de ScenarioData.
 */
struct Operation {
    int id = -1;                    // Identificador de la operación (normalmente 0,1,2... por job)

    Operation() = default;          // Constructor por defecto → id = -1 (inválido)
    
    /**
     * @brief Constructor explícito con ID
     * @param _id Identificador de la operación
     */
    explicit Operation(int _id) : id(_id) {}
};

/**
 * @brief Representa un trabajo (job) completo en el problema de scheduling
 * 
 * Un Job consiste en una secuencia ordenada de operaciones que deben ejecutarse
 * en ese orden (precedencia estricta entre operaciones del mismo job).
 */
struct Job {
    int id = -1;                    // Identificador del trabajo (normalmente 0,1,2,...,numJobs-1)
    
    QVector<int> operations;        // Lista ordenada de IDs de operaciones que componen este job
                                    // El orden en el vector indica la precedencia requerida

    Job() = default;                // Constructor por defecto → id = -1, operations vacío
    
    /**
     * @brief Constructor explícito con ID
     * @param _id Identificador del trabajo
     */
    explicit Job(int _id) : id(_id) {}

    /**
     * @brief Método auxiliar para añadir una operación al final de la secuencia
     * @param opId Identificador de la operación a añadir
     */
    void addOperation(int opId) {
        operations.append(opId);
    }
};

#endif // SCENARIO_TYPES_H