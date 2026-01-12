#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "EvolutionaryAlgorithm.h"
#include "scenarioloader.h"
#include "ScenarioData.h"

#include <QFile>
#include <QQueue>
#include <QRegularExpression>
#include <QtCharts/QChartView>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QFileDialog>
#include <QTableWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>


void clearWidget(QWidget* widget) {
    if (!widget) return;

    // El layout del widget
    QLayout* layout = widget->layout();
    if (layout) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();  // borra widget hijo
            }
            delete item; // borra el item del layout
        }
        delete layout;  // borra el layout
    }
}

void parseHyperparametersFromString(
    const QString& str,
    QVector<float>& mutationRates,
    float& crossoverRate)
{
    // Quitar corchetes y espacios
    QString clean = str;
    clean.remove('[');
    clean.remove(']');
    clean = clean.trimmed();

    // Separar por coma
    QStringList tokens = clean.split(',', Qt::SkipEmptyParts);

    if (tokens.size() != 4) {
        qWarning() << "Error: se esperaban 4 valores en el string";
        return;
    }

    // Convertir a float
    bool ok;
    mutationRates[ReciprocalExchange] = tokens[0].toFloat(&ok);
    if (!ok) qWarning() << "Error al convertir ReciprocalExchange";

    mutationRates[Shift] = tokens[1].toFloat(&ok);
    if (!ok) qWarning() << "Error al convertir Shift";

    mutationRates[InterChromosome] = tokens[2].toFloat(&ok);
    if (!ok) qWarning() << "Error al convertir InterChromosome";

    crossoverRate = tokens[3].toFloat(&ok);
    if (!ok) qWarning() << "Error al convertir crossoverRate";
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->FIFOButton, &QRadioButton::toggled, this, &MainWindow::onPolicyToggled);
    connect(ui->LTPButton,    &QRadioButton::toggled, this, &MainWindow::onPolicyToggled);
    connect(ui->STPButton,    &QRadioButton::toggled, this, &MainWindow::onPolicyToggled);
    connect(ui->RRFIFOButton, &QRadioButton::toggled, this, &MainWindow::onPolicyToggled);
    connect(ui->RRLTPButton,  &QRadioButton::toggled, this, &MainWindow::onPolicyToggled);
    connect(ui->RRECAButton,  &QRadioButton::toggled, this, &MainWindow::onPolicyToggled);


    QSettings settings("config.ini", QSettings::IniFormat);
    apiKey = settings.value("API/GeminiKey", "").toString();

    if (apiKey.isEmpty()) {
        qDebug() << "ERROR: API key no encontrada en config.ini";
    }

}

QString MainWindow::openTxtFile(QWidget *parent)
{
    QString filePath = QFileDialog::getOpenFileName(
        parent,
        "Seleccionar archivo de texto",
        QString(),                      // directorio inicial
        "Archivos de texto (*.txt)"     // filtro
        );

    if (filePath.isEmpty()) {
        return QString(); // Usuario canceló
    }

    // Verificación extra (por seguridad)
    if (!filePath.endsWith(".txt", Qt::CaseInsensitive)) {
        QMessageBox::warning(parent,
                             "Archivo inválido",
                             "Debe seleccionar un archivo .txt");
        return QString();
    }

    return filePath;
}

void MainWindow::plotPareto(const QVector<Individual> &population, QWidget *widget)
{
    if (population.isEmpty()) return;

    // -------------------------
    // LIMPIAR WIDGET
    // -------------------------
    if (widget->layout()) {
        QLayoutItem *item;
        while ((item = widget->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete widget->layout();
    }

    int numChrom = population[0].chromosomes.size();

    // -------------------------
    // COLORES (equivalente a {"r","g","b","m","c","y","k"})
    // -------------------------
    QVector<QColor> colors = {
        Qt::red, Qt::green, Qt::blue,
        Qt::magenta, Qt::cyan, Qt::yellow, Qt::black
    };

    // -------------------------
    // CHART
    // -------------------------
    QChart *chart = new QChart();
    chart->setTitle("Población - Distribución de Makespan vs Energía");
    chart->legend()->setVisible(true);

    double minF1 = std::numeric_limits<double>::max();
    double maxF1 = std::numeric_limits<double>::lowest();
    double minF2 = std::numeric_limits<double>::max();
    double maxF2 = std::numeric_limits<double>::lowest();

    // -------------------------
    // CREAR SERIES POR CROMOSOMA
    // -------------------------
    for (int c = 0; c < numChrom; ++c) {

        QScatterSeries *series = new QScatterSeries();
        series->setMarkerSize(10.0);

        QColor color = colors[c % colors.size()];
        color.setAlpha(150);
        series->setColor(color);
        series->setBorderColor(color);


        // Etiqueta tipo: "Cromosoma FIFO", "Cromosoma SPT", etc.
        series->setName("Cromosoma " + population[0].chromosomes[c].policyName);

        for (const Individual &ind : population) {
            const auto &chrom = ind.chromosomes[c];

            minF1 = std::min(minF1, chrom.f1);
            maxF1 = std::max(maxF1, chrom.f1);
            minF2 = std::min(minF2, chrom.f2);
            maxF2 = std::max(maxF2, chrom.f2);

            // SOLO FRENTE DE PARETO
            if (chrom.domLevel == 1) {
                series->append(chrom.f1, chrom.f2);
            }
        }

        chart->addSeries(series);
    }

    // -------------------------
    // EJES
    // -------------------------
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Makespan");
    axisX->setLabelFormat("%.2f");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Energía");
    axisY->setLabelFormat("%.2f");

    double marginX = (maxF1 - minF1) * 0.05;
    double marginY = (maxF2 - minF2) * 0.05;

    axisX->setRange(minF1 - marginX, maxF1 + marginX);
    axisY->setRange(minF2 - marginY, maxF2 + marginY);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // -------------------------
    // ASOCIAR EJES A TODAS LAS SERIES
    // -------------------------
    for (QAbstractSeries *s : chart->series()) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    // -------------------------
    // CHART VIEW
    // -------------------------
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // -------------------------
    // INSERTAR EN WIDGET
    // -------------------------
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);

    widget->setLayout(layout);
    widget->setMinimumHeight(600);
}

void MainWindow::plotPopulation(const QVector<Individual> &population, QWidget *widget)
{
    if (population.isEmpty()) return;

    // -------------------------
    // LIMPIAR WIDGET
    // -------------------------
    if (widget->layout()) {
        QLayoutItem *item;
        while ((item = widget->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete widget->layout();
    }

    int numChrom = population[0].chromosomes.size();

    // -------------------------
    // COLORES
    // -------------------------
    QVector<QColor> colors = {
        Qt::red, Qt::green, Qt::blue,
        Qt::magenta, Qt::cyan, Qt::yellow, Qt::black
    };

    // -------------------------
    // CHART
    // -------------------------
    QChart *chart = new QChart();
    chart->setTitle("Población Total - Distribución de Makespan vs Energía");
    chart->legend()->setVisible(true);

    double minF1 = std::numeric_limits<double>::max();
    double maxF1 = std::numeric_limits<double>::lowest();
    double minF2 = std::numeric_limits<double>::max();
    double maxF2 = std::numeric_limits<double>::lowest();

    // -------------------------
    // SERIES POR CROMOSOMA
    // -------------------------
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

            // POBLACIÓN COMPLETA
            series->append(chrom.f1, chrom.f2);
        }

        chart->addSeries(series);
    }

    // -------------------------
    // EJES
    // -------------------------
    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Makespan");
    axisX->setLabelFormat("%.2f");

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Energía");
    axisY->setLabelFormat("%.2f");

    double marginX = (maxF1 - minF1) * 0.05;
    double marginY = (maxF2 - minF2) * 0.05;

    axisX->setRange(minF1 - marginX, maxF1 + marginX);
    axisY->setRange(minF2 - marginY, maxF2 + marginY);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    // -------------------------
    // ASOCIAR EJES
    // -------------------------
    for (QAbstractSeries *s : chart->series()) {
        s->attachAxis(axisX);
        s->attachAxis(axisY);
    }

    // -------------------------
    // CHART VIEW
    // -------------------------
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    // -------------------------
    // INSERTAR EN WIDGET
    // -------------------------
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);

    widget->setLayout(layout);
    widget->setMinimumHeight(600);
}

QString MainWindow::buildGeneticPrompt(
    const QString& contentScenario,
    const QString& last10Hypervolumes,
    const Individual& kneePoint,
    const Individual& bestMakespan,
    const Individual& bestEnergy,
    const QVector<float>& mutationRates,
    float crossoverRate)
{
    // ============================
    // Construir el prompt
    // ============================
    QString prompt = QString(R"(Eres un asistente experto en optimización de algoritmos genéticos para planificación de tareas.
    Tu tarea es sugerir los hiperparámetros de un algoritmo genético basado en los resultados de cada generación en un escenario de máquinas y operaciones.

    El escenario es el siguiente:

    %1

    A continuación, te proporcionaré los resultados de la evolución del algoritmo genético y los anteriores hiperparámetros:

    - Lista de hipervolúmenes por generación: %2
    - Punto de rodilla: %3
    - Mejor makespan: %4
    - Mejor energía: %5
    - MutationRate InterChromosome: %6
    - MutationRate ReciprocalExchange: %7
    - MutationRate Shift: %8
    - CrossoverRate: %9

    Con base en estos resultados, sugiere los valores óptimos de los hiperparámetros para la siguiente ejecución del algoritmo genético:

    - MutationRate para InterChromosome, ReciprocalExchange, Shift
    - CrossoverRate

    Devuelve únicamente una lista con 4 valores, en el siguiente orden y formato:

    [InterChromosome, ReciprocalExchange, Shift, crossoverRate]

    Las mutaciones puede ir de %10 a %11 y el crossoverRate puede ir de %12 a %13

    No agregues explicaciones ni texto adicional. Solo devuelve la lista con los valores sugeridos.)")
                         .arg(contentScenario)
                         .arg(last10Hypervolumes)
                         .arg(kneePoint.toString(true))
                         .arg(bestMakespan.toString(true))
                         .arg(bestEnergy.toString(true))
                         .arg(mutationRates[InterChromosome])
                         .arg(mutationRates[ReciprocalExchange])
                         .arg(mutationRates[Shift])
                         .arg(crossoverRate)
                         .arg(ui->inputMinMut->value())
                         .arg(ui->inputMaxMut->value())
                         .arg(ui->inputMinCross->value())
                         .arg(ui->inputMaxCross->value());

    return prompt;
}


void MainWindow::on_startButton_clicked()
{
    ui->progressBar->setValue(0);
    clearWidget(ui->tablesWidget);
    QRandomGenerator rng;

    if (path.isEmpty()) {
        QMessageBox::warning(
            this,
            "Archivo no seleccionado",
            "Debe seleccionar un archivo de texto (.txt) antes de continuar."
            );
        return;
    }

    if (policyNames.isEmpty()) {
        QMessageBox::critical(
            this,
            "Políticas no seleccionadas",
            "Debe seleccionar al menos una política para iniciar el algoritmo genético."
            );
        return;
    }

    scenario = loadScenario(path, policyNames);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir el archivo:" << path;
        return;
    }

    QTextStream in(&file);
    QString contentScenario = in.readAll();  // Lee todo el contenido
    file.close();

    int populationSize = ui->popSizeInput->value();
    qDebug() << "Tamaño de la Poblacion:" << populationSize;

    if (populationSize == 0) {
        QMessageBox::critical(
            this,
            "Valor inválido",
            "El tamaño de la población debe ser mayor que 0."
            );
        return;
    }

    int numGen = ui->numGenInput->value();
    qDebug() << "Numero de Generaciones:" << numGen;

    int numGenLLM = ui->inputNumGenLLM->value();

    if (numGen == 0) {
        QMessageBox::critical(
            this,
            "Valor inválido",
            "El número de generaciones debe ser mayor que 0."
            );
        return;
    }

    float crossoverRate = ui->inputInitialCross->value();

    QVector<float> mutationRates(MutationCount);
    mutationRates[InterChromosome]      = ui->inputInitialMutInter->value();
    mutationRates[ReciprocalExchange]   = ui->inputInitialMutRE->value();
    mutationRates[Shift]                = ui->inputInitialMutShift->value();



    EvolutionaryAlgorithm ea(
        scenario,
        policyNames,
        populationSize,
        crossoverRate,
        mutationRates
        );

    ea.initialize();

    QVector<Individual> pop = ea.getPopulation();
    QVector<QVector<double>> hypervolumes = ea.getHypervolumes();
    QString last20Str = hypervolumeStringLast20(hypervolumes, policyNames);
    Individual kneePoint = ea.getKneePoint();
    Individual bestMakespan = ea.getBestMakespan();
    Individual bestEnergy = ea.getBestEnergy();

    QString prompt;

    plotPareto(pop, ui->plotParetoFirstWidget);
    plotPopulation(pop, ui->plotPopulationFirstWidget);
    addHypervolumeTable(hypervolumes, 0, policyNames);

    for(int gen=1; gen<=numGen; gen++){
        ea.runGeneration();
        ui->progressBar->setValue(gen*numGen/100);
        if (gen % numGenLLM == 0 && gen!=numGen) {
            hypervolumes = ea.getHypervolumes();
            addHypervolumeTable(hypervolumes, gen, policyNames);

            last20Str = hypervolumeStringLast20(hypervolumes, policyNames);
            kneePoint = ea.getKneePoint();
            bestMakespan = ea.getBestMakespan();
            bestEnergy = ea.getBestEnergy();
            prompt = buildGeneticPrompt(contentScenario, last20Str, kneePoint, bestMakespan, bestEnergy, mutationRates, crossoverRate);

            qDebug() << "Prompt:" << prompt;

            QString response = callGeminiAPI(prompt, apiKey, ui->inputTemp->value());

            qDebug() << "Respuesta del modelo:" << response;
            parseHyperparametersFromString(response, mutationRates, crossoverRate);
        }
    }

    pop = ea.getPopulation();
    hypervolumes = ea.getHypervolumes();

    ui->progressBar->setValue(100);
    addHypervolumeTable(hypervolumes, numGen, policyNames);
    plotHypervolumeEvolution(hypervolumes, policyNames);
    plotPareto(pop, ui->plotParetoFinalWidget);
    plotPopulation(pop, ui->plotPopulationFinalWidget);

    kneePoint = ea.getKneePoint();
    bestMakespan = ea.getBestMakespan();
    bestEnergy = ea.getBestEnergy();

    plotGanttChart(scenario, bestMakespan, ui->ganttMakespan, ea);
    plotGanttChart(scenario, kneePoint, ui->ganttKneePoint, ea);
    plotGanttChart(scenario, bestEnergy, ui->ganttEnergy, ea);
}

void MainWindow::on_fileButton_clicked()
{
    path = openTxtFile(this);

    if (path.isEmpty())
        return;

    qDebug() << "Archivo seleccionado:" << path;

}

void MainWindow::onPolicyToggled(bool checked)
{
    QRadioButton *btn = qobject_cast<QRadioButton*>(sender());
    if (!btn) return;

    QString policy = btn->text(); // O btn->property("policy").toString();

    if (checked)
        policyNames.append(policy);
    else
        policyNames.removeAll(policy);
}

void MainWindow::plotHypervolumeEvolution(
    const QVector<QVector<double>>& hypervolumes,
    const QVector<QString>& policyNames)
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

    // =============================
    // AGREGAR AL LAYOUT
    // =============================
    if (!ui->tablesWidget->layout()) {
        ui->tablesWidget->setLayout(new QVBoxLayout());
    }

    chartView->setMinimumHeight(600);
    ui->tablesWidget->layout()->addWidget(chartView);
}

void MainWindow::addHypervolumeTable(
    const QVector<QVector<double>>& hypervolumes,
    int generation,
    const QVector<QString>& policyNames)
{
    if (hypervolumes.isEmpty())
        return;

    int numPolicies = policyNames.size();

    // =============================
    // TITULO DE LA GENERACION
    // =============================
    QLabel* title = new QLabel(
        QString("Generación %1").arg(generation));
    title->setStyleSheet("font-weight: bold; font-size: 14px;");
    title->setAlignment(Qt::AlignCenter);

    // =============================
    // TABLA
    // =============================
    QTableWidget* table = new QTableWidget(numPolicies, 4);
    table->setHorizontalHeaderLabels(
        {"Política", "Min", "Max", "Promedio"});

    // =============================
    // CALCULO POR POLITICA
    // =============================
    for (int c = 0; c < numPolicies; ++c) {

        QVector<double> hvPerPolicy;

        // recorrer generaciones hasta la actual
        for (int g = 0; g < hypervolumes.size(); ++g) {
            if (c < hypervolumes[g].size())
                hvPerPolicy.append(hypervolumes[g][c]);
        }

        if (hvPerPolicy.isEmpty())
            continue;

        double minVal = *std::min_element(hvPerPolicy.begin(), hvPerPolicy.end());
        double maxVal = *std::max_element(hvPerPolicy.begin(), hvPerPolicy.end());
        double mean   = std::accumulate(
                          hvPerPolicy.begin(),
                          hvPerPolicy.end(), 0.0)
                      / hvPerPolicy.size();

        table->setItem(c, 0, new QTableWidgetItem(policyNames[c]));
        table->setItem(c, 1, new QTableWidgetItem(QString::number(minVal, 'f', 4)));
        table->setItem(c, 2, new QTableWidgetItem(QString::number(maxVal, 'f', 4)));
        table->setItem(c, 3, new QTableWidgetItem(QString::number(mean, 'f', 4)));
    }

    table->resizeColumnsToContents();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setMinimumHeight(25+30*numPolicies);

    // =============================
    // AGREGAR AL LAYOUT
    // =============================
    if (!ui->tablesWidget->layout()) {
        ui->tablesWidget->setLayout(new QVBoxLayout());
    }

    ui->tablesWidget->layout()->addWidget(title);
    ui->tablesWidget->layout()->addWidget(table);
}

QString MainWindow::hypervolumeStringLast20(
    const QVector<QVector<double>>& hypervolumes,
    const QVector<QString>& policyNames)
{
    if (hypervolumes.isEmpty() || policyNames.isEmpty())
        return QString();

    int numPolicies = policyNames.size();
    QString result;

    // Recorrer cada política
    for (int c = 0; c < numPolicies; ++c) {
        QVector<double> last20;

        // Tomar hasta los últimos 20 de la política c
        for (int g = qMax<qsizetype>(0, hypervolumes.size() - 20); g < hypervolumes.size(); ++g) {
            if (c < hypervolumes[g].size())
                last20.append(hypervolumes[g][c]);
        }

        // Convertir QVector<double> a QString separado por comas
        QStringList strList;
        for (double val : last20)
            strList << QString::number(val, 'f', 4);

        result += QString("%1: [%2]\n")
                      .arg(policyNames[c])
                      .arg(strList.join(", "));
    }

    return result;
}

QString MainWindow::callGeminiAPI(const QString &prompt,
                                  const QString &apiKey,
                                  double temperature)
{
    QNetworkAccessManager manager;

    QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", apiKey.toUtf8());

    // Construir JSON
    QJsonObject partObj;
    partObj["text"] = prompt;

    QJsonArray partsArray;
    partsArray.append(partObj);

    QJsonObject contentObj;
    contentObj["parts"] = partsArray;

    QJsonArray contentsArray;
    contentsArray.append(contentObj);

    QJsonObject generationConfig;
    generationConfig["temperature"] = temperature;

    QJsonObject rootObj;
    rootObj["contents"] = contentsArray;
    rootObj["generationConfig"] = generationConfig;

    QJsonDocument doc(rootObj);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = manager.post(request, data);

    // ⚡ Esperar a que termine la respuesta
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();  // bloquea aquí hasta que reply termine

    QString generatedText;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error en request:" << reply->errorString();
    } else {
        QByteArray responseData = reply->readAll();

        // Parse JSON
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        if (jsonDoc.isObject()) {
            QJsonObject obj = jsonDoc.object();
            QJsonArray candidates = obj["candidates"].toArray();
            if (!candidates.isEmpty()) {
                QJsonObject firstCandidate = candidates[0].toObject();
                QJsonObject content = firstCandidate["content"].toObject();
                QJsonArray parts = content["parts"].toArray();
                if (!parts.isEmpty()) {
                    generatedText = parts[0].toObject()["text"].toString();
                }
            }
        }
    }

    reply->deleteLater();
    return generatedText;  // retorna el texto directamente
}

void MainWindow::plotGanttChart(
    const ScenarioData& scenario,
    const Individual ind,
    QWidget *widget,
    EvolutionaryAlgorithm& ea)
{
    // Limpiar widget previo
    if (widget->layout()) {
        QLayoutItem* item;
        while ((item = widget->layout()->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete widget->layout();
    }
    QVBoxLayout* layout = new QVBoxLayout(widget);
    for (Chromosome chromosome : ind.chromosomes){
        QLabel* title = new QLabel(chromosome.policyName);
        title->setStyleSheet("font-weight: bold; font-size: 14px;");
        title->setAlignment(Qt::AlignCenter);

        const QVector<OperationSchedule>& schedule = ea.evaluateChromosome(chromosome);
        // Crear scene y view
        QGraphicsScene* scene = new QGraphicsScene();
        QGraphicsView* view = new QGraphicsView(scene);
        view->setRenderHint(QPainter::Antialiasing);

        const double yStep = 40.0;      // altura por máquina
        const double rectHeight = 30.0; // altura del rectángulo
        const double timeScale = 10.0;  // escala de tiempo (multiplicador para que no quede muy pequeño)

        QMap<int, QColor> jobColors; // colores por Job
        QRandomGenerator rng(QRandomGenerator::global()->generate());

        // Dibujar operaciones
        for (const OperationSchedule& op : schedule) {
            // color por Job
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
            double height = rectHeight;

            QGraphicsRectItem* rect = scene->addRect(x, y, width, height, QPen(Qt::black), QBrush(jobColors[op.jobId]));

            // Etiqueta: JobId/OpId
            QGraphicsTextItem* label = scene->addText(
                QString("J%1-O%2").arg(op.jobId).arg(op.operationId),
                QFont("Arial", 8)
                );
            label->setPos(x + 2, y + 2);
        }

        // Etiquetas de máquinas
        for (int m = 0; m < scenario.numMachines; ++m) {
            QGraphicsTextItem* machineLabel = scene->addText(
                QString("Máquina %1").arg(m),
                QFont("Arial", 10, QFont::Bold)
                );
            machineLabel->setPos(0, m * yStep - 20);
        }

        // Ajustar view
        scene->setSceneRect(0, 0, 1000, scenario.numMachines * yStep + 50); // ancho dinámico aproximado
        view->setMinimumHeight(scenario.numMachines * yStep + 50);

        layout->setContentsMargins(0,0,0,0);
        layout->addWidget(title);
        layout->addWidget(view);
    }
    widget->setMinimumHeight(600*ind.getNumChromosomes());
    widget->setLayout(layout);
}


MainWindow::~MainWindow()
{
    delete ui;
}
