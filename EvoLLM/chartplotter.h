#ifndef CHARTPLOTTER_H
#define CHARTPLOTTER_H

#include <QtCharts>
#include <QWidget>
#include "Individual.h"   // donde esté definido Individual
#include "scenariodata.h"
#include "EvolutionaryAlgorithm.h"

class ChartPlotter
{
public:
    static void plotPareto(const QVector<Individual> &population, QWidget *widget);
    static void plotPopulation(const QVector<Individual> &population, QWidget *widget);
    static void plotHypervolumeEvolution(const QVector<QVector<double>>& hypervolumes, const QVector<QString>& policyNames, QWidget *widget);
    static void plotParameterEvolution(const QVector<QVector<double>>& parameters,QWidget *widget);
    static void plotGanttChart(const ScenarioData& scenario, const Individual ind, QWidget *containerWidget, EvolutionaryAlgorithm& ea);

private:
    static void clearWidget(QWidget *widget);
};

#endif // CHARTPLOTTER_H
