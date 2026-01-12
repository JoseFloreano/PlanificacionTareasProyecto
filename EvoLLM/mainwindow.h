#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "individual.h"
#include "scenariodata.h"
#include "scheduletypes.h"
#include "EvolutionaryAlgorithm.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_startButton_clicked();
    void on_fileButton_clicked();
    void onPolicyToggled(bool checked);

private:
    Ui::MainWindow *ui;
    QString apiKey;
    QString path = "";
    ScenarioData scenario;
    QVector<QString> policyNames = {"FIFO"};

    void plotPopulation(const QVector<Individual> &pop, QWidget *widget);
    void plotPareto(const QVector<Individual> &pop, QWidget * widget);
    void plotGanttChart(const ScenarioData& scenario, const Individual ind, QWidget *containerWidget, EvolutionaryAlgorithm& ea);
    void plotHypervolumeEvolution(const QVector<QVector<double>>& hypervolumes, const QVector<QString>& policyNames);
    void addHypervolumeTable(const QVector<QVector<double>>& hypervolumes, int generation, const QVector<QString>& policyNames);

    QString openTxtFile(QWidget *parent);
    QString hypervolumeStringLast20(const QVector<QVector<double>>& hypervolumes, const QVector<QString>& policyNames);

    QString buildGeneticPrompt(const QString& contentScenario,const QString& last10Hypervolumes,const Individual& kneePoint, const Individual& bestMakespan,
                               const Individual& bestEnergy, const QVector<float>& mutationRates, float crossoverRate);
    QString callGeminiAPI(const QString &prompt, const QString &apiKey, double temperature = 0.7);

};
#endif // MAINWINDOW_H
