#ifndef CHROMOSOME_H
#define CHROMOSOME_H

#include <QString>              // Para manejar nombres de políticas como cadenas
#include <QVector>              // Contenedor para la secuencia de genes
#include <QRandomGenerator>     // Generador de números aleatorios (usado en inicialización)

/**
 * @brief Representa una solución candidata (individuo) en un algoritmo evolutivo
 * 
 * En este contexto, cada Chromosome representa una posible solución para un problema
 * de scheduling multiobjetivo (normalmente Makespan vs Consumo Energético).
 * 
 * La codificación genética (genes) es un vector de enteros cuya interpretación depende
 * del tipo de problema y de la política de codificación/decodificación utilizada.
 */
class Chromosome {
public:
    QString policyName;             // Nombre de la política/estrategia/heurística que representa este cromosoma
    QVector<int> genes;             // Secuencia de genes (valores enteros) que codifican la solución
    
    double f1;                      // Objetivo 1: típicamente Makespan (tiempo total de finalización)
    double f2;                      // Objetivo 2: típicamente Consumo total de energía
    
    int domLevel;                   // Nivel de dominancia (usado en algoritmos tipo NSGA-II)
                                    // -1 = no calculado, 1 = frente de Pareto, >1 = dominado
    
    double crowdingDistance;        // Distancia de crowding (diversidad en NSGA-II)
                                    // -1 = no calculada, mayor valor = más diversa en su frente

    // Constructores
    /**
     * @brief Constructor por defecto
     * Inicializa un cromosoma vacío con valores por defecto
     */
    Chromosome();

    /**
     * @brief Constructor con nombre de política
     * @param name Nombre de la política/heurística que representa esta solución
     */
    explicit Chromosome(const QString& name);

    // Métodos
    /**
     * @brief Inicializa el vector de genes con valores enteros aleatorios
     * @param size Longitud deseada del cromosoma (número de genes)
     * @param minValue Valor mínimo posible para cada gen
     * @param maxValue Valor máximo posible para cada gen
     * @param rng Generador de números aleatorios (se pasa por referencia)
     */
    void initializeRandom(int size, int minValue, int maxValue, QRandomGenerator& rng);

    /**
     * @brief Devuelve el número actual de genes en el cromosoma
     * @return Tamaño del vector genes
     */
    int size() const;

    /**
     * @brief Genera una representación legible del cromosoma como texto
     * Útil para depuración, logs o mostrar información al usuario
     * @return QString con formato: "NombrePolítica: [gen1, gen2, gen3, ...]"
     */
    QString toString() const;
};

#endif // CHROMOSOME_H