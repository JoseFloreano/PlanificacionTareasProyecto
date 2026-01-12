#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "EvolutionaryAlgorithm.h"
#include "scenarioloader.h"
#include "ScenarioData.h"
#include "chartplotter.h"

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

void MainWindow::on_startButton_clicked()
{
    ui->progressBar->setValue(0);
    clearWidget(ui->tablesWidget);
    clearWidget(ui->hyperWidget);
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
    QVector<QVector<double>> parameters;

    QString last20Str = hypervolumeStringLast20(hypervolumes, policyNames);
    Individual kneePoint = ea.getKneePoint();
    Individual bestMakespan = ea.getBestMakespan();
    Individual bestEnergy = ea.getBestEnergy();

    QString prompt;

    ChartPlotter::plotPareto(pop, ui->plotParetoFirstWidget);
    ChartPlotter::plotPopulation(pop, ui->plotPopulationFirstWidget);
    addHypervolumeTable(hypervolumes, 0, policyNames);

    for(int gen=1; gen<=numGen; gen++){
        parameters.append({mutationRates[InterChromosome], mutationRates[ReciprocalExchange], mutationRates[Shift], crossoverRate});
        ea.runGeneration();
        ui->progressBar->setValue(gen*numGen/100);
        if (gen % numGenLLM == 0 && gen!=numGen) {
            hypervolumes = ea.getHypervolumes();
            QVector<float> prevMutationRates(MutationCount);
            prevMutationRates[InterChromosome]      = mutationRates[InterChromosome];
            prevMutationRates[ReciprocalExchange]   = mutationRates[ReciprocalExchange];
            prevMutationRates[Shift]                = mutationRates[Shift];
            float prevCrossoverRate = crossoverRate;

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

            addHyperparameterTable(gen,prevCrossoverRate,prevMutationRates,crossoverRate,mutationRates);
        }
    }

    pop = ea.getPopulation();
    hypervolumes = ea.getHypervolumes();

    ui->progressBar->setValue(100);
    addHypervolumeTable(hypervolumes, numGen, policyNames);
    ChartPlotter::plotHypervolumeEvolution(hypervolumes, policyNames, ui->tablesWidget);
    ChartPlotter::plotParameterEvolution(parameters, ui->hyperWidget);
    ChartPlotter::plotPareto(pop, ui->plotParetoFinalWidget);
    ChartPlotter::plotPopulation(pop, ui->plotPopulationFinalWidget);

    kneePoint = ea.getKneePoint();
    bestMakespan = ea.getBestMakespan();
    bestEnergy = ea.getBestEnergy();

    ChartPlotter::plotGanttChart(scenario, bestMakespan, ui->ganttMakespan, ea);
    ChartPlotter::plotGanttChart(scenario, kneePoint, ui->ganttKneePoint, ea);
    ChartPlotter::plotGanttChart(scenario, bestEnergy, ui->ganttEnergy, ea);

    showBestIndividualsSummary(bestMakespan, bestEnergy, kneePoint,ui->resultsWidget);
}

void MainWindow::on_fileButton_clicked()
{
    path = openTxtFile(this);

    if (path.isEmpty())
        return;

    qDebug() << "Archivo seleccionado:" << path;

}

void MainWindow::on_exportCsvButton_clicked()
{
    if (csvInd.isEmpty()) {
        QMessageBox::warning(
            this,
            "No hay resultados generados",
            "Primero genera resultados lol."
            );
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Exportar resultados",
        "",
        "CSV (*.csv)"
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << csvInd;
        file.close();
    }
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

void MainWindow::addHyperparameterTable(
    int generation,
    float initialCrossover,
    const QVector<float>& initialMutationRates,
    float modelCrossover,
    const QVector<float>& modelMutationRates)
{
    // =============================
    // TITULO
    // =============================
    QLabel* title = new QLabel(
        QString("Hiperparámetros – Generación %1").arg(generation));
    title->setStyleSheet("font-weight: bold; font-size: 14px;");
    title->setAlignment(Qt::AlignCenter);

    // =============================
    // TABLA
    // =============================
    QTableWidget* table = new QTableWidget(4, 3);
    table->setHorizontalHeaderLabels(
        {"Parámetro", "Inicial", "Modelo"});

    table->verticalHeader()->setVisible(false);

    // =============================
    // FILAS
    // =============================
    struct Row {
        QString name;
        float initVal;
        float modelVal;
    };

    QVector<Row> rows = {
        {"CrossoverRate", initialCrossover, modelCrossover},
        {"Mut.InterChromosome",
         initialMutationRates[InterChromosome],
         modelMutationRates[InterChromosome]},
        {"Mut.ReciprocalExchange",
         initialMutationRates[ReciprocalExchange],
         modelMutationRates[ReciprocalExchange]},
        {"Mut.Shift",
         initialMutationRates[Shift],
         modelMutationRates[Shift]}
    };

    for (int r = 0; r < rows.size(); ++r) {
        table->setItem(r, 0, new QTableWidgetItem(rows[r].name));
        table->setItem(r, 1, new QTableWidgetItem(QString::number(rows[r].initVal, 'f', 3)));
        table->setItem(r, 2, new QTableWidgetItem(QString::number(rows[r].modelVal, 'f', 3)));
    }

    table->resizeColumnsToContents();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setMinimumHeight(25 + 30 * rows.size());

    // =============================
    // AGREGAR AL LAYOUT
    // =============================
    if (!ui->hyperWidget->layout()) {
        ui->hyperWidget->setLayout(new QVBoxLayout());
    }

    ui->hyperWidget->layout()->addWidget(title);
    ui->hyperWidget->layout()->addWidget(table);
}

void MainWindow::showBestIndividualsSummary(
    const Individual &bestMakespan,
    const Individual &bestEnergy,
    const Individual &kneePoint,
    QWidget *widget)
{
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

    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setSpacing(20);

    // -------------------------
    // FUNCIÓN AUXILIAR
    // -------------------------
    auto addIndividualTable =
        [&](const Individual &ind, const QString &titleText)
    {
        QLabel *title = new QLabel(titleText);
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-weight: bold; font-size: 15px;");

        int numChrom = ind.getNumChromosomes();

        QTableWidget *table = new QTableWidget(numChrom, 4);
        table->setHorizontalHeaderLabels(
            {"Política", "Genes", "Makespan (f1)", "Energía (f2)"}
            );

        for (int c = 0; c < numChrom; ++c) {
            const Chromosome &chrom = ind.chromosomes[c];

            table->setItem(c, 0,
                           new QTableWidgetItem(chrom.policyName));

            table->setItem(c, 1,
                           new QTableWidgetItem(chrom.toString()));

            table->setItem(c, 2,
                           new QTableWidgetItem(
                               QString::number(chrom.f1, 'f', 4)));

            table->setItem(c, 3,
                           new QTableWidgetItem(
                               QString::number(chrom.f2, 'f', 4)));
        }

        table->resizeColumnsToContents();
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->setMinimumHeight(30 + numChrom * 30);

        mainLayout->addWidget(title);
        mainLayout->addWidget(table);
    };

    // -------------------------
    // AGREGAR LOS 3 INDIVIDUOS
    // -------------------------
    addIndividualTable(bestMakespan, "Mejor Makespan");
    addIndividualTable(bestEnergy,   "Mejor Energía");
    addIndividualTable(kneePoint,    "Punto de Rodilla");

    widget->setLayout(mainLayout);

    QTextStream out(&csvInd);

    // Header
    out << "IndividualType,Policy,Genes,Makespan,Energia\n";

    // Función auxiliar
    auto appendIndividual =
        [&](const Individual &ind, const QString &label)
    {
        for (const Chromosome &chrom : ind.chromosomes) {

            // Genes como string seguro para CSV
            QString genes = chrom.toString();
            genes.replace("\"", "\"\""); // escape por si acaso

            out << label << ","
                << chrom.policyName << ","
                << "\"" << genes << "\"" << ","
                << chrom.f1 << ","
                << chrom.f2 << "\n";
        }
    };

    appendIndividual(bestMakespan, "BestMakespan");
    appendIndividual(bestEnergy,   "BestEnergy");
    appendIndividual(kneePoint,    "KneePoint");
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

MainWindow::~MainWindow()
{
    delete ui;
}
