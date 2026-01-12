#ifndef SCENARIO_LOADER_H
#define SCENARIO_LOADER_H

#include <QString>
#include <QVector>
#include <QQueue>
#include <QPair>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <stdexcept>
#include <algorithm>

#include "ScenarioData.h"

// -------------------------
// Funciones auxiliares
// -------------------------
bool isCommentOrEmpty(const QString& line);
QVector<double> parseLineToDoubles(const QString& line);
QVector<int> parseJobOperations(const QString& line);

// -------------------------
// Función principal
// -------------------------
ScenarioData loadScenario(const QString& filename,
                          const QVector<QString>& policyNames);

#endif // SCENARIO_LOADER_H
