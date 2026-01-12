#ifndef SCENARIO_DATA_H
#define SCENARIO_DATA_H

#include <QVector>
#include <QMap>
#include <QString>
#include "ScenarioTypes.h"

struct ScenarioData {
    int numOperations;     // Operaciones únicas
    int totalOperations;   // Total de operaciones considerando todos los jobs
    int numMachines;
    int numJobs;

    QVector<QVector<double>> processingTime;
    QVector<QVector<double>> energyCost;
    QVector<Job> jobs;

    QMap<QString, QVector<QPair<Job, Operation>>> chromosomeMapping;

    ScenarioData()
        : numOperations(0),
        totalOperations(0),
        numMachines(0),
        numJobs(0) {}
};

#endif // SCENARIO_DATA_H
