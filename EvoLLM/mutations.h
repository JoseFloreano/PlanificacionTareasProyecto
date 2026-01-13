#ifndef MUTATIONS_H
#define MUTATIONS_H

#include <random>                   // Para std::mt19937 y distribuciones aleatorias
#include "individual.h"             // Clase Individual (contiene múltiples cromosomas)

/**
 * @file Mutations.h
 * @brief Conjunto de funciones de mutación específicas para el algoritmo evolutivo
 * 
 * Todas las mutaciones operan directamente sobre un objeto Individual,
 * afectando uno o varios de sus cromosomas según el tipo de mutación.
 * 
 * Cada función recibe:
 * - El individuo a mutar (por referencia)
 * - Generador de números aleatorios (Mersenne Twister)
 * - Tasa de mutación específica para ese operador
 * - Distribución uniforme [0,1) para decidir si aplicar la mutación
 */

/**
 * @brief Mutación entre cromosomas (Inter-Chromosome)
 * 
 * Intercambia material genético (genes) entre diferentes cromosomas del mismo individuo.
 * Representa una transferencia de conocimiento o estrategia entre políticas distintas.
 * 
 * Típicamente útil cuando se quiere fomentar cooperación o intercambio de buenas ideas
 * entre diferentes codificaciones/heurísticas.
 */
void mutationInterChromosome(Individual& individual,
                            std::mt19937& rng,
                            float mutationRate,
                            std::uniform_real_distribution<double>& dist);

/**
 * @brief Mutación por intercambio recíproco (Reciprocal Exchange)
 * 
 * Dentro de un mismo cromosoma, selecciona dos posiciones y las intercambia.
 * Clásica mutación de intercambio usada en problemas de permutación o secuencias.
 * 
 * Muy común en problemas de scheduling donde el orden importa (job shop, flow shop, etc.).
 */
void mutationReciprocalExchange(Individual& individual,
                               std::mt19937& rng,
                               float mutationRate,
                               std::uniform_real_distribution<double>& dist);

/**
 * @brief Mutación por desplazamiento (Shift)
 * 
 * Selecciona un gen y lo desplaza hacia otra posición dentro del mismo cromosoma,
 * desplazando los genes intermedios para mantener la longitud.
 * 
 * Similar a una inserción/eliminación en una posición, útil para alterar ligeramente
 * secuencias manteniendo la mayoría de la estructura original.
 */
void mutationShift(Individual& individual,
                  std::mt19937& rng,
                  float mutationRate,
                  std::uniform_real_distribution<double>& dist);

#endif // MUTATIONS_H