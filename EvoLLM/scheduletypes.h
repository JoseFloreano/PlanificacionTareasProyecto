#ifndef SCHEDULE_TYPES_H
#define SCHEDULE_TYPES_H

/**
 * @file ScheduleTypes.h
 * @brief Definiciones de estructuras utilizadas durante la simulación y evaluación
 * del scheduling (programación real de operaciones en máquinas)
 * 
 * Estas estructuras son usadas principalmente en la fase de decodificación/evaluación
 * cuando se transforma un cromosoma (solución codificada) en una programación concreta.
 */

/**
 * @brief Representa el estado actual de una máquina durante la simulación de scheduling
 * 
 * Se utiliza para llevar el control temporal y energético de cada máquina
 * mientras se van programando las operaciones.
 */
struct MachineState {
    double currentTime = 0.0;       // Momento actual en el que la máquina está libre
                                    // (el tiempo más temprano en que puede empezar una nueva operación)

    double totalEnergy = 0.0;       // Consumo energético acumulado por esta máquina hasta el momento

    bool isActive = false;          // Indica si la máquina está ocupada en este instante
                                    // (aunque en la mayoría de implementaciones se deduce del currentTime)
};

/**
 * @brief Representa una operación que ha sido programada en una máquina específica
 * 
 * Es el resultado final de la decodificación: cada operación de cada trabajo
 * termina con una instancia de esta estructura cuando se completa la evaluación.
 */
struct OperationSchedule {
    int operationId = -1;           // Identificador de la operación dentro de su job
    int jobId = -1;                 // Identificador del trabajo al que pertenece
    int machineId = -1;             // Máquina en la que se ejecutó esta operación

    double startTime = 0.0;         // Instante de inicio real de la operación
    double endTime = 0.0;           // Instante de finalización real
    double processingTime = 0.0;    // Tiempo de procesamiento efectivo (endTime - startTime)
    double energyCost = 0.0;        // Consumo energético real de esta operación en la máquina elegida
};

/**
 * @brief Representa el estado de progreso de un trabajo (job) durante la simulación
 * 
 * Ayuda a controlar qué operación debe ejecutarse a continuación y cuándo estará
 * disponible el job para su siguiente operación (debido a precedencias).
 */
struct JobState {
    int nextOperationIndex = 0;     // Índice de la siguiente operación que debe programarse
                                    // (0 = primera operación pendiente, operations.size() = terminado)

    double lastOperationEndTime = 0.0;  // Momento en que terminó la operación anterior del mismo job
                                        // Define el earliest start time de la siguiente operación
};

#endif // SCHEDULE_TYPES_H