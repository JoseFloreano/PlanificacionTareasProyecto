#ifndef CHARTPLOTTER_H
#define CHARTPLOTTER_H

#include <QtCharts>             // Módulo necesario para QChart, QChartView, series, ejes, etc.
#include <QWidget>              // Clase base para los widgets donde se mostrarán las gráficas
#include "Individual.h"         // Donde está definida la estructura Individual (y Chromosome)
#include "scenariodata.h"       // Contiene la información del problema (máquinas, jobs, etc.)
#include "EvolutionaryAlgorithm.h" // Clase que contiene la lógica de evaluación del algoritmo evolutivo

/**
 * @brief Clase estática encargada de generar diferentes tipos de visualizaciones
 * relacionadas con el algoritmo evolutivo multiobjetivo (Makespan vs Energía)
 * 
 * Todas las funciones son estáticas, por lo que no se necesita instanciar la clase.
 * Todas las gráficas se dibujan dentro del QWidget que se pasa como parámetro.
 */
class ChartPlotter
{
public:
    /**
     * @brief Muestra el frente de Pareto actual (solo soluciones no dominadas)
     * Una serie scatter por cada política/cromosoma
     * 
     * @param population Población completa con todos los individuos evaluados
     * @param widget Widget Qt donde se dibujará la gráfica
     */
    static void plotPareto(const QVector<Individual> &population, QWidget *widget);

    /**
     * @brief Muestra TODA la población actual (no solo el frente de Pareto)
     * Útil para observar la diversidad y distribución general de soluciones
     * 
     * @param population Población completa
     * @param widget Widget donde se renderizará el gráfico
     */
    static void plotPopulation(const QVector<Individual> &population, QWidget *widget);

    /**
     * @brief Gráfica de líneas que muestra la evolución del hipervolumen
     * a lo largo de las generaciones para cada política
     * 
     * @param hypervolumes Vector donde hypervolumes[g][p] = hipervolumen de la política p en generación g
     * @param policyNames Nombres de las políticas/cromosomas para la leyenda
     * @param widget Contenedor donde se mostrará la gráfica
     */
    static void plotHypervolumeEvolution(
        const QVector<QVector<double>>& hypervolumes,
        const QVector<QString>& policyNames,
        QWidget *widget);

    /**
     * @brief Visualiza la evolución de los hiperparámetros adaptativos
     * (probabilidades de mutación y crossover) a lo largo de las generaciones
     * 
     * @param parameters parameters[g][p] = valor del hiperparámetro p en generación g
     * @param widget Widget donde se dibujará la evolución
     */
    static void plotParameterEvolution(
        const QVector<QVector<double>>& parameters,
        QWidget *widget);

    /**
     * @brief Genera uno o varios diagramas de Gantt (uno por política/cromosoma)
     * mostrando la programación detallada de operaciones en las máquinas
     * 
     * @param scenario Datos del problema (jobs, operaciones, máquinas, tiempos...)
     * @param ind Individuo/solución a visualizar (contiene varios cromosomas/políticas)
     * @param containerWidget Widget donde se colocará el área con scroll con los Gantt
     * @param ea Instancia del algoritmo evolutivo (necesaria para evaluar el cromosoma)
     */
    static void plotGanttChart(
        const ScenarioData& scenario,
        const Individual ind,
        QWidget *containerWidget,
        EvolutionaryAlgorithm& ea);

private:
    /**
     * @brief Función auxiliar que limpia completamente un widget
     * eliminando su layout y todos sus hijos (útil antes de crear una nueva gráfica)
     * 
     * @param widget Widget que se desea limpiar
     */
    static void clearWidget(QWidget *widget);
};

#endif // CHARTPLOTTER_H