#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>
#include <QString>
#include <QRandomGenerator>
#include "Chromosome.h"

class Individual {
public:
    QVector<Chromosome> chromosomes;

    // Constructor: crea cromosomas a partir de nombres de políticas
    explicit Individual(const QVector<QString>& policyNames);

    // Inicializa todos los cromosomas con valores aleatorios
    void initializeRandom(int chromosomeSize,
                          int minValue,
                          int maxValue,
                          QRandomGenerator& rng);

    // Número de cromosomas
    int getNumChromosomes() const;

    // Representación completa del individuo
    QString toString(bool showFitness = false) const;

    // Validación estructural del individuo
    bool isValid() const;
};

#endif // INDIVIDUAL_H
