#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include <QVector>
#include <QString>
#include <QRandomGenerator>
#include "Chromosome.h"

/**
 * @brief Representa un individuo completo en el algoritmo evolutivo multi-política
 * 
 * Un Individual contiene múltiples cromosomas, uno por cada política/codificación/heurística
 * que se está evaluando simultáneamente en el algoritmo multiobjetivo.
 * 
 * Cada cromosoma representa una solución candidata bajo una política específica,
 * pero todas forman parte del mismo "individuo" para evaluación y selección conjunta.
 */
class Individual {
public:
    QVector<Chromosome> chromosomes;    // Uno por cada política en uso
                                        // Cada uno tiene su propia codificación genética

    /**
     * @brief Constructor principal
     * Crea un individuo con un cromosoma por cada política indicada
     * 
     * @param policyNames Lista de nombres de políticas (cada una tendrá su cromosoma)
     */
    explicit Individual(const QVector<QString>& policyNames);

    /**
     * @brief Inicializa TODOS los cromosomas del individuo con valores aleatorios
     * 
     * @param chromosomeSize Longitud de cada cromosoma (número de genes)
     * @param minValue Valor mínimo permitido para cada gen
     * @param maxValue Valor máximo permitido para cada gen
     * @param rng Generador de números aleatorios (se pasa por referencia)
     */
    void initializeRandom(int chromosomeSize,
                         int minValue,
                         int maxValue,
                         QRandomGenerator& rng);

    /**
     * @brief Devuelve cuántos cromosomas contiene este individuo
     * (equivalente al número de políticas en uso)
     */
    int getNumChromosomes() const;

    /**
     * @brief Genera una representación textual completa del individuo
     * Útil para depuración, logs o visualización
     * 
     * @param showFitness Si true, también muestra los valores de f1 y f2 de cada cromosoma
     * @return QString con información de todos los cromosomas
     */
    QString toString(bool showFitness = false) const;

    /**
     * @brief Verifica si la estructura del individuo es válida
     * 
     * Actualmente comprueba que:
     * - Hay al menos un cromosoma
     * - Todos los cromosomas tienen el mismo tamaño
     * - Ningún cromosoma está vacío
     * 
     * @return true si el individuo parece estructuralmente correcto
     */
    bool isValid() const;
};

#endif // INDIVIDUAL_H