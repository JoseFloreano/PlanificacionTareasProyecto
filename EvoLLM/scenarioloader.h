#ifndef SCENARIO_LOADER_H
#define SCENARIO_LOADER_H

#include <QString>
#include <QVector>
#include <QQueue>
#include <QPair>
#include <QDebug>               // Para mensajes de depuración (qDebug())
#include <QFile>                // Manejo de archivos
#include <QTextStream>          // Lectura/escritura de texto en archivos
#include <QRegularExpression>   // Expresiones regulares para parseo
#include <stdexcept>            // Para lanzar excepciones (std::runtime_error)
#include <algorithm>            // Para std::sort
#include "ScenarioData.h"       // Estructura que contiene todos los datos del problema

// -------------------------
// Funciones auxiliares
// -------------------------

/**
 * @brief Determina si una línea del archivo es un comentario o está vacía
 * 
 * Se considera comentario cualquier línea que, después de eliminar espacios,
 * comienza con '#' o está completamente vacía.
 * 
 * @param line Línea leída del archivo
 * @return true si es comentario o línea vacía, false en caso contrario
 */
bool isCommentOrEmpty(const QString& line);

/**
 * @brief Convierte una línea de texto en un vector de valores double
 * 
 * Divide la línea por espacios (uno o más) y convierte cada token a double.
 * Ignora partes vacías.
 * 
 * @param line Línea con valores numéricos separados por espacios
 * @return QVector<double> con los valores parseados
 */
QVector<double> parseLineToDoubles(const QString& line);

/**
 * @brief Parsea la definición de operaciones de un trabajo
 * 
 * Espera formato similar a:  Job1 {O1, O2, O3, O4}
 * Extrae los números de operación (O1 → 0, O2 → 1, etc.) y los devuelve
 * como índices base 0.
 * 
 * @param line Línea que contiene la secuencia de operaciones de un job
 * @return QVector<int> con los índices de operaciones (base 0)
 */
QVector<int> parseJobOperations(const QString& line);

// -------------------------
// Función principal
// -------------------------

/**
 * @brief Carga una instancia completa del problema desde un archivo de texto
 * 
 * Lee y parsea el archivo en tres secciones principales:
 * 1. Tiempos de procesamiento por operación y máquina
 * 2. Costos energéticos por operación y máquina
 * 3. Definición de trabajos y sus secuencias de operaciones
 * 
 * Además, genera los mapeos de secuencia de operaciones (chromosomeMapping)
 * para cada política indicada (FIFO, LTP, RRFIFO, etc.)
 * 
 * @param filename Ruta completa al archivo que contiene la instancia
 * @param policyNames Lista de nombres de políticas para las cuales generar mapeos
 * @return ScenarioData estructura completamente inicializada y lista para usar
 * @throws std::runtime_error si hay problemas al abrir el archivo o formato inválido
 */
ScenarioData loadScenario(const QString& filename,
                         const QVector<QString>& policyNames);

#endif // SCENARIO_LOADER_H