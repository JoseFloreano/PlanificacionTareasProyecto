#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <QString>
#include <QVector>
#include <QRandomGenerator>

class Chromosome {
public:
    QString policyName;
    QVector<int> genes;
    double f1;               // Makespan
    double f2;               // Energía total
    int domLevel;            // Nivel de dominancia
    double crowdingDistance; // Distancia de Crowding

    // Constructores
    Chromosome();
    explicit Chromosome(const QString& name);

    // Métodos
    void initializeRandom(int size, int minValue, int maxValue, QRandomGenerator& rng);
    int size() const;
    QString toString() const;
};

#endif // CHROMOSOME_H
