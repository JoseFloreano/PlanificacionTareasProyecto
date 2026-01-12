#include "Individual.h"

// Constructor: crea cromosomas a partir del vector de nombres
Individual::Individual(const QVector<QString>& policyNames) {
    chromosomes.reserve(policyNames.size());

    for (const QString& name : policyNames) {
        chromosomes.append(Chromosome(name));
    }
}

// Inicializa todos los cromosomas con valores aleatorios
void Individual::initializeRandom(int chromosomeSize,
                                  int minValue,
                                  int maxValue,
                                  QRandomGenerator& rng) {
    for (Chromosome& chromosome : chromosomes) {
        chromosome.initializeRandom(chromosomeSize, minValue, maxValue, rng);
    }
}

// Número de cromosomas
int Individual::getNumChromosomes() const {
    return chromosomes.size();
}

// Representación completa del individuo
QString Individual::toString(bool showFitness) const {
    QString result;
    result += "INDIVIDUO POLIPLOIDE\n";
    result += QString(50, '=') + "\n";

    for (const Chromosome& chromosome : chromosomes) {
        result += chromosome.toString() + "\n";
    }

    if (showFitness) {
        result += "\nFitness:\n";
        for (const Chromosome& chromosome : chromosomes) {
            result += QString("%1 -> f1: %2, f2: %3\n")
            .arg(chromosome.policyName)
                .arg(chromosome.f1)
                .arg(chromosome.f2);
        }
    }

    result += QString(50, '=') + "\n";
    return result;
}

// Valida la estructura del individuo
bool Individual::isValid() const {
    if (chromosomes.isEmpty()) {
        return false;
    }

    int expectedSize = chromosomes.first().size();

    for (const Chromosome& chromosome : chromosomes) {
        // Todos deben tener el mismo tamaño
        if (chromosome.size() != expectedSize) {
            return false;
        }

        // Todos los genes deben ser positivos
        for (int gene : chromosome.genes) {
            if (gene <= 0) {
                return false;
            }
        }
    }

    return true;
}
