#include "ChartPlotter.h"
#include <limits>

// --------------------------------
// LIMPIAR WIDGET
// --------------------------------
void ChartPlotter::clearWidget(QWidget *widget)
{
    if (widget->layout()) {
        QLayoutItem *item;
        while ((item = widget->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete widget->layout();
    }
}

void ChartPlotter::plotPareto(const QVector<Individual> &population, QWidget *widget)
{
    if (population.isEmpty()) return;

    clearWidget(widget);

    int numChrom = population[0].chromosomes.size();

    QVector<QColor> colors = {
        Qt::red, Qt::green, Qt::blue,
        Qt::magenta, Qt::cyan, Qt::yellow, Qt::black
    };

    QChart *chart = new QChart();
    chart->setTitle("Frente de Pareto - Makespan vs Energía");
    chart->legend()->setVisible(true);

    double minF1 = std::numeric_limits<double>::max();
    double maxF1 = std::numeric_limits<double>::lowest();
    double minF2 = std::numeric_limits<double>::max();
    double maxF2 = std::numeric_limits<double>::lowest();

    for (int c = 0; c < numChrom; ++c) {

        QScatterSeries *series = new QScatterSeries();
        series->setMarkerSize(10.0);

        QColor color = colors[c % colors.size()];
        color.setAlpha(150);
        series->setColor(color);
        series->setBorderColor(color);

        series->setName("Cromosoma " + population[0].chromosomes[c].policyName);

        for (const Individual &ind : population) {
            const auto &chrom = ind.chromosomes[c];

            minF1 = std::min(minF1, chrom.f1);
            maxF1 = std::max(maxF1, chrom.f1);
            minF2 = std::min(minF2, chrom.f2);
            maxF2 = std::max(maxF2, chrom.f2);

            if (chrom.domLevel == 1) {
                series->append(chrom.f1, chrom.f2);
            }
        }

        chart->addSeries(series);
    }

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Makespan");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Energía");

    double marginX = (maxF1 - minF1) * 0.05;
    double marginY = (maxF2 - minF2) * 0.05;

    axisX->setRange(minF1 - marginX, maxF1 + marginX);
    axisY->setRange(minF2 - marginY, maxF2 + marginY);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (QAbstractSeries *s : chart->series()) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(600);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    widget->setLayout(layout);
}

void ChartPlotter::plotPopulation(const QVector<Individual> &population, QWidget *widget)
{
    if (population.isEmpty()) return;

    clearWidget(widget);

    int numChrom = population[0].chromosomes.size();

    QVector<QColor> colors = {
        Qt::red, Qt::green, Qt::blue,
        Qt::magenta, Qt::cyan, Qt::yellow, Qt::black
    };

    QChart *chart = new QChart();
    chart->setTitle("Población Total - Makespan vs Energía");
    chart->legend()->setVisible(true);

    double minF1 = std::numeric_limits<double>::max();
    double maxF1 = std::numeric_limits<double>::lowest();
    double minF2 = std::numeric_limits<double>::max();
    double maxF2 = std::numeric_limits<double>::lowest();

    for (int c = 0; c < numChrom; ++c) {

        QScatterSeries *series = new QScatterSeries();
        series->setMarkerSize(8.0);

        QColor color = colors[c % colors.size()];
        color.setAlpha(120);
        series->setColor(color);
        series->setBorderColor(color);

        series->setName("Cromosoma " + population[0].chromosomes[c].policyName);

        for (const Individual &ind : population) {
            const auto &chrom = ind.chromosomes[c];

            minF1 = std::min(minF1, chrom.f1);
            maxF1 = std::max(maxF1, chrom.f1);
            minF2 = std::min(minF2, chrom.f2);
            maxF2 = std::max(maxF2, chrom.f2);

            series->append(chrom.f1, chrom.f2);
        }

        chart->addSeries(series);
    }

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Makespan");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Energía");

    double marginX = (maxF1 - minF1) * 0.05;
    double marginY = (maxF2 - minF2) * 0.05;

    axisX->setRange(minF1 - marginX, maxF1 + marginX);
    axisY->setRange(minF2 - marginY, maxF2 + marginY);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (QAbstractSeries *s : chart->series()) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(600);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    widget->setLayout(layout);
}


void ChartPlotter::plotHypervolumeEvolution(
    const QVector<QVector<double>>& hypervolumes,
    const QVector<QString>& policyNames,
    QWidget *widget)
{
    if (hypervolumes.isEmpty() || policyNames.isEmpty())
        return;

    int numPolicies = policyNames.size();
    int numGenerations = hypervolumes.size();

    // Crear QChart
    QChart* chart = new QChart();
    chart->setTitle("Evolución del Hipervolumen por Política");

    // Crear series para cada política
    QVector<QLineSeries*> seriesList;
    for (int c = 0; c < numPolicies; ++c) {
        QLineSeries* series = new QLineSeries();
        series->setName(policyNames[c]);

        // Agregar puntos de todas las generaciones
        for (int g = 0; g < numGenerations; ++g) {
            if (c < hypervolumes[g].size()) {
                series->append(g, hypervolumes[g][c]);
            }
        }

        chart->addSeries(series);
        seriesList.append(series);
    }

    // Configurar ejes
    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Generación");
    axisX->setLabelFormat("%d");
    axisX->setRange(0, numGenerations-1);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Hipervolumen");

    // Calcular min y max del hipervolumen para el eje Y
    double minHV = hypervolumes[0][0];
    double maxHV = hypervolumes[0][0];
    for (const auto& genHV : hypervolumes) {
        for (double hv : genHV) {
            if (hv < minHV) minHV = hv;
            if (hv > maxHV) maxHV = hv;
        }
    }
    axisY->setRange(minHV*0.95, maxHV*1.05); // un poco de margen
    axisY->setLabelFormat("%.4f");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Adjuntar ejes a cada serie
    for (QLineSeries* s : seriesList) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    // Mostrar en QChartView dentro de un widget
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    if (!widget->layout()) {
        widget->setLayout(new QVBoxLayout());
    }

    chartView->setMinimumHeight(600);
    widget->layout()->addWidget(chartView);
}

void ChartPlotter::plotParameterEvolution(
    const QVector<QVector<double>>& parameters,
    QWidget *widget)
{
    if (parameters.isEmpty())
        return;

    int numGenerations = parameters.size();
    int numParams = parameters[0].size(); // debería ser 4

    if (numParams < 4)
        return;

    // -------------------------
    // NOMBRES Y COLORES
    // -------------------------
    QVector<QString> paramNames = {
        "Mut. InterChromosome",
        "Mut. ReciprocalExchange",
        "Mut. Shift",
        "CrossoverRate"
    };

    QVector<QColor> colors = {
        Qt::red,
        Qt::blue,
        Qt::green,
        Qt::magenta
    };

    // -------------------------
    // CHART
    // -------------------------
    QChart *chart = new QChart();
    chart->setTitle("Evolución de Hiperparámetros");
    chart->legend()->setVisible(true);

    QVector<QLineSeries*> seriesList;

    // -------------------------
    // CREAR SERIES
    // -------------------------
    for (int p = 0; p < numParams; ++p) {

        QLineSeries *series = new QLineSeries();
        series->setName(paramNames[p]);

        QPen pen(colors[p % colors.size()]);
        pen.setWidth(2);
        series->setPen(pen);

        for (int g = 0; g < numGenerations; ++g) {
            series->append(g, parameters[g][p]);
        }

        chart->addSeries(series);
        seriesList.append(series);
    }

    // -------------------------
    // EJES
    // -------------------------
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Generación");
    axisX->setLabelFormat("%d");
    axisX->setRange(0, numGenerations - 1);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Valor del parámetro");
    axisY->setLabelFormat("%.2f");
    axisY->setRange(0.0, 1.0); // hiperparámetros normalizados

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // -------------------------
    // ASOCIAR EJES
    // -------------------------
    for (QLineSeries *s : seriesList) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    // -------------------------
    // CHART VIEW
    // -------------------------
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(600);
    widget->layout()->addWidget(chartView);

}

void ChartPlotter::plotGanttChart(
    const ScenarioData& scenario,
    const Individual ind,
    QWidget *widget,
    EvolutionaryAlgorithm& ea)
{
    // -------------------------
    // LIMPIAR WIDGET
    // -------------------------
    if (widget->layout()) {
        QLayoutItem* item;
        while ((item = widget->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete widget->layout();
    }

    // -------------------------
    // SCROLL AREA PRINCIPAL
    // -------------------------
    QScrollArea *scrollArea = new QScrollArea(widget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *container = new QWidget();
    QGridLayout *grid = new QGridLayout(container);
    grid->setSpacing(15);
    grid->setContentsMargins(10, 10, 10, 10);

    // -------------------------
    // PARÁMETROS GANTT
    // -------------------------
    const double yStep = 40.0;
    const double rectHeight = 30.0;
    const double timeScale = 10.0;

    int row = 0;

    // -------------------------
    // UN GANTT POR CROMOSOMA
    // -------------------------
    for (Chromosome chromosome : ind.chromosomes) {

        // -------- TÍTULO --------
        QLabel *title = new QLabel(chromosome.policyName);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-weight: bold; font-size: 14px;");

        grid->addWidget(title, row, 0);
        row++;

        // -------- EVALUAR --------
        const QVector<OperationSchedule> &schedule =
            ea.evaluateChromosome(chromosome);

        // -------- SCENE / VIEW --------
        QGraphicsScene *scene = new QGraphicsScene();
        scene->setBackgroundBrush(Qt::white);

        QGraphicsView *view = new QGraphicsView(scene);
        view->setRenderHint(QPainter::Antialiasing);
        view->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        QMap<int, QColor> jobColors;
        QRandomGenerator rng(QRandomGenerator::global()->generate());

        double maxEndTime = 0.0;

        // -------- DIBUJAR OPERACIONES --------
        for (const OperationSchedule &op : schedule) {

            if (!jobColors.contains(op.jobId)) {
                jobColors[op.jobId] = QColor::fromRgb(
                    rng.bounded(50, 256),
                    rng.bounded(50, 256),
                    rng.bounded(50, 256)
                    );
            }

            double x = op.startTime * timeScale;
            double y = op.machineId * yStep;
            double width = op.processingTime * timeScale;

            maxEndTime = std::max(maxEndTime, op.endTime);

            scene->addRect(
                x, y,
                width, rectHeight,
                QPen(Qt::black),
                QBrush(jobColors[op.jobId])
                );

            QGraphicsTextItem *label = scene->addText(
                QString("J%1-O%2").arg(op.jobId).arg(op.operationId),
                QFont("Arial", 8)
                );
            label->setPos(x + 3, y + 3);
        }

        // -------- ETIQUETAS DE MÁQUINAS --------
        for (int m = 0; m < scenario.numMachines; ++m) {
            QGraphicsTextItem *machineLabel = scene->addText(
                QString("M%1").arg(m),
                QFont("Arial", 9, QFont::Bold)
                );
            machineLabel->setPos(-40, m * yStep);
        }

        // -------- SCENE SIZE --------
        double sceneWidth = maxEndTime * timeScale + 100;
        double sceneHeight = scenario.numMachines * yStep + 30;

        scene->setSceneRect(0, 0, sceneWidth, sceneHeight);
        view->setMinimumHeight(sceneHeight + 10);
        view->setMinimumWidth(600);

        grid->addWidget(view, row, 0);
        row++;
    }

    container->setLayout(grid);
    scrollArea->setWidget(container);

    // -------------------------
    // LAYOUT FINAL
    // -------------------------
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scrollArea);

    widget->setLayout(mainLayout);
    widget->setMinimumHeight(600);
}
