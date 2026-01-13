#include "ScenarioLoader.h"

// -------------------------
// Funciones auxiliares
// -------------------------

// Determina si una línea es comentario o está vacía
bool isCommentOrEmpty(const QString& line) {
    QString trimmed = line.trimmed();
    return trimmed.isEmpty() || trimmed.startsWith('#');
}

// Convierte una línea de texto en vector de doubles (valores numéricos separados por espacios)
QVector<double> parseLineToDoubles(const QString& line) {
    QVector<double> values;
    QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString& token : tokens) {
        values.append(token.toDouble());
    }
    return values;
}

// Parsea la línea que define las operaciones de un trabajo (formato: {O1, O2, O3, ...})
QVector<int> parseJobOperations(const QString& line) {
    QVector<int> operations;
    int start = line.indexOf('{');
    int end = line.indexOf('}');
    if (start == -1 || end == -1 || end <= start) {
        return operations; // Formato inválido → retornamos vacío
    }
    
    QString content = line.mid(start + 1, end - start - 1);
    QStringList tokens = content.split(',', Qt::SkipEmptyParts);
    
    for (QString token : tokens) {
        token = token.trimmed();
        int pos = token.indexOf('O');
        if (pos != -1 && pos + 1 < token.length()) {
            bool ok = false;
            int opNum = token.mid(pos + 1).toInt(&ok);
            if (ok) operations.append(opNum - 1); // Convertimos a índice base 0
        }
    }
    return operations;
}

// -------------------------
// Función principal: Cargar escenario completo desde archivo
// -------------------------
ScenarioData loadScenario(
    const QString& filename,
    const QVector<QString>& policyNames
) {
    ScenarioData data;
    QFile file(filename);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error(
            QString("ERROR: No se pudo abrir el archivo: %1")
                .arg(filename).toStdString()
        );
    }

    QTextStream in(&file);
    QString line;
    int section = 0;          // 0 = ninguna, 1 = tiempos, 2 = energía, 3 = trabajos
    int rowCount = 0;
    
    qDebug() << "CARGANDO ESCENARIO DESDE:" << filename;

    // Estructuras temporales para ordenar trabajos según criterios
    QVector<QPair<Job,double>> jobsWithTimes;   // Para LTP/STP
    QVector<QPair<Job,double>> jobsWithEnergy;  // Para políticas energéticas

    while (!in.atEnd()) {
        line = in.readLine();

        // Saltamos líneas vacías o comentarios
        if (isCommentOrEmpty(line)) {
            QString trimmed = line.trimmed();
            // Detectamos cambio de sección por palabras clave
            if (trimmed.contains("tiempos", Qt::CaseInsensitive)) {
                section = 1; rowCount = 0;
            } else if (trimmed.contains("energ", Qt::CaseInsensitive)) {
                section = 2; rowCount = 0;
            } else if (trimmed.contains("trabajo", Qt::CaseInsensitive)) {
                section = 3;
            }
            continue;
        }

        // Sección 1: Tiempos de procesamiento (una fila por operación)
        if (section == 1) {
            QVector<double> times = parseLineToDoubles(line);
            data.numMachines = times.size();           // Primera fila determina # máquinas
            data.processingTime.append(times);
            rowCount++;
            data.numOperations++;                      // Cada fila = una operación única
        }
        // Sección 2: Costos energéticos (misma estructura que tiempos)
        else if (section == 2) {
            QVector<double> energy = parseLineToDoubles(line);
            if (energy.size() != data.numMachines) {
                throw std::runtime_error("ERROR: Fila de energia con numero incorrecto de maquinas");
            }
            data.energyCost.append(energy);
            rowCount++;
        }
        // Sección 3: Definición de trabajos y sus secuencias de operaciones
        else if (section == 3) {
            QVector<int> ops = parseJobOperations(line);
            if (!ops.isEmpty()) {
                Job job(data.numJobs);           // ID del job = índice actual
                job.operations = ops;
                data.jobs.push_back(job);
                data.numJobs++;
            }
        }
    }
    file.close();

    // Calculamos el total real de operaciones (suma de ops de todos los jobs)
    int totalOp = 0;
    for (const auto& job : data.jobs) totalOp += job.operations.size();
    data.totalOperations = totalOp;

    // Calculamos tiempos y energía promedio por job (para ordenamientos posteriores)
    for (const Job& job : data.jobs) {
        double totalTime = 0.0;
        double totalEnergy = 0.0;
        for (int op : job.operations) {
            double t = 0.0, e = 0.0;
            for (int m = 0; m < data.numMachines; ++m) {
                t += data.processingTime[op][m];
                e += data.energyCost[op][m];
            }
            totalTime += t / data.numMachines;     // Promedio por máquina
            totalEnergy += e / data.numMachines;
        }
        jobsWithTimes.push_back({job, totalTime});
        jobsWithEnergy.push_back({job, totalEnergy});
    }

    // ===============================
    // Generación de mapeos para cada POLÍTICA
    // ===============================
    for (const QString& policy : policyNames) {
        data.chromosomeMapping[policy].clear();

        // Cola por job para políticas Round-Robin
        QVector<QQueue<QPair<Job, Operation>>> rr(data.numJobs);

        // FIFO: orden secuencial simple por orden de jobs y operaciones
        if (policy == "FIFO") {
            for (const Job& job : data.jobs)
                for (int op : job.operations)
                    data.chromosomeMapping[policy].push_back({job, Operation(op)});
        }
        // LTP (Longest Total Processing time) / STP (Shortest)
        else if (policy == "LTP" || policy == "STP") {
            std::sort(jobsWithTimes.begin(), jobsWithTimes.end(),
                      [&](const auto& a, const auto& b){
                          return policy == "LTP" ? a.second > b.second : a.second < b.second;
                      });
            for (const auto& pair : jobsWithTimes)
                for (int op : pair.first.operations)
                    data.chromosomeMapping[policy].push_back({pair.first, Operation(op)});
        }
        // Políticas Round-Robin: RRFIFO, RRLTP, RRECA
        else if (policy == "RRFIFO" || policy == "RRLTP" || policy == "RRECA") {
            // Preparar colas por job según criterio
            if (policy == "RRFIFO") {
                for (int i = 0; i < data.numJobs; ++i)
                    for (int op : data.jobs[i].operations)
                        rr[i].enqueue({data.jobs[i], Operation(op)});
            }
            else if (policy == "RRLTP") {
                std::sort(jobsWithTimes.begin(), jobsWithTimes.end(),
                          [](auto& a, auto& b){ return a.second > b.second; });
                for (int i = 0; i < data.numJobs; ++i)
                    for (int op : jobsWithTimes[i].first.operations)
                        rr[i].enqueue({jobsWithTimes[i].first, Operation(op)});
            }
            else { // RRECA (probablemente Round-Robin Energy Consumption Awareness)
                std::sort(jobsWithEnergy.begin(), jobsWithEnergy.end(),
                          [](auto& a, auto& b){ return a.second < b.second; });
                for (int i = 0; i < data.numJobs; ++i)
                    for (int op : jobsWithEnergy[i].first.operations)
                        rr[i].enqueue({jobsWithEnergy[i].first, Operation(op)});
            }

            // Algoritmo Round-Robin: toma una operación de cada job activo por turno
            int active = rr.size();
            while (active > 0) {
                for (int i = 0; i < rr.size(); ++i) {
                    if (!rr[i].isEmpty()) {
                        data.chromosomeMapping[policy].push_back(rr[i].dequeue());
                    } else {
                        rr.removeAt(i);
                        --active;
                        --i; // Reajustamos índice tras eliminación
                    }
                }
            }
        }
    }

    qDebug() << "Escenario cargado exitosamente";
    qDebug() << "Numero de Maquinas:" << data.numMachines;
    qDebug() << "Numero de Operaciones:" << data.numOperations;
    qDebug() << "Numero de Trabajos:" << data.numJobs;
    qDebug() << "Total de Operaciones:" << data.totalOperations;

    return data;
}