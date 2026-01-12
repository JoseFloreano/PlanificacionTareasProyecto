#include "Chromosome.h"

// Constructor por defecto
Chromosome::Chromosome()
    : policyName(""),
    f1(0.0),
    f2(0.0),
    domLevel(-1),
    crowdingDistance(-1) {
}

// Constructor con nombre de política
Chromosome::Chromosome(const QString& name)
    : policyName(name),
    f1(0.0),
    f2(0.0),
    domLevel(-1),
    crowdingDistance(-1) {
}

// Inicializa el cromosoma con valores aleatorios
void Chromosome::initializeRandom(int size, int minValue, int maxValue, QRandomGenerator& rng) {
    genes.clear();
    genes.reserve(size);

    for (int i = 0; i < size; ++i) {
        genes.append(rng.bounded(minValue, maxValue + 1));
    }
}

// Obtiene el tamaño del cromosoma
int Chromosome::size() const {
    return genes.size();
}

// Convierte el cromosoma a QString legible
QString Chromosome::toString() const {
    QString result = policyName + ": [";

    for (int i = 0; i < genes.size(); ++i) {
        result += QString::number(genes[i]);
        if (i < genes.size() - 1)
            result += ", ";
    }

    result += "]";
    return result;
}
