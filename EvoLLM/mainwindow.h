#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "individual.h"             // Clase que representa un individuo (varios cromosomas)
#include "scenariodata.h"           // Estructura con los datos del problema de scheduling
#include "scheduletypes.h"          // Tipos relacionados con el scheduling (OperationSchedule, etc.)
#include "EvolutionaryAlgorithm.h"  // El núcleo del algoritmo evolutivo multiobjetivo

#include <QMainWindow>              // Clase base de ventana principal en Qt

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;                   // Clase generada automáticamente por Qt Designer
}
QT_END_NAMESPACE

/**
 * @brief Ventana principal de la aplicación
 * 
 * Gestiona la interfaz gráfica, interacción con el usuario,
 * ejecución del algoritmo evolutivo, visualización de resultados
 * y la integración con la API de Gemini para sugerencias adaptativas.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    /**
     * @brief Inicia o detiene la ejecución del algoritmo evolutivo
     * (botón principal "Iniciar"/"Detener")
     */
    void on_startButton_clicked();

    /**
     * @brief Abre diálogo para seleccionar archivo de instancia del problema
     * (JSON, TXT, etc. según implementación)
     */
    void on_fileButton_clicked();

    /**
     * @brief Exporta resultados relevantes (población, hipervolúmenes, mejores soluciones...)
     * a formato CSV para análisis posterior
     */
    void on_exportCsvButton_clicked();

    /**
     * @brief Maneja el cambio de estado de los checkboxes de selección de políticas
     * Permite activar/desactivar políticas dinámicamente
     */
    void onPolicyToggled(bool checked);

private:
    Ui::MainWindow *ui;                 // Interfaz generada por Qt Designer

    QString apiKey;                     // Clave para la API de Google Gemini
    QString path = "";                  // Ruta del archivo de instancia cargado actualmente
    QString csvInd = "";                // (Posiblemente nombre o contenido para exportación CSV)

    ScenarioData scenario;              // Datos del problema actualmente cargados

    QVector<QString> policyNames = {"FIFO"};  // Políticas activas (inicialmente solo FIFO)
                                              // Se pueden añadir más dinámicamente

    // Métodos auxiliares para actualización de la interfaz

    /**
     * @brief Añade/actualiza la tabla de evolución de hipervolúmenes
     * en la interfaz (normalmente una QTableWidget)
     */
    void addHypervolumeTable(const QVector<QVector<double>>& hypervolumes,
                            int generation,
                            const QVector<QString>& policyNames);

    /**
     * @brief Actualiza la tabla que muestra la evolución de hiperparámetros
     * (crossover y tasas de mutación) en cada generación
     */
    void addHyperparameterTable(int generation,
                               float initialCrossover,
                               const QVector<float>& initialMutationRates,
                               float modelCrossover,
                               const QVector<float>& modelMutationRates);

    /**
     * @brief Muestra un resumen de los mejores individuos encontrados:
     * - Mejor en Makespan
     * - Mejor en Energía
     * - Punto de rodilla (trade-off recomendado)
     */
    void showBestIndividualsSummary(const Individual &bestMakespan,
                                   const Individual &bestEnergy,
                                   const Individual &kneePoint,
                                   QWidget *widget);

    /**
     * @brief Abre un diálogo para seleccionar y leer un archivo de texto
     * @return Contenido completo del archivo como QString
     */
    QString openTxtFile(QWidget *parent);

    /**
     * @brief Genera un string con los últimos 20 valores de hipervolumen
     * (formato legible para mostrar o enviar a la API)
     */
    QString hypervolumeStringLast20(const QVector<QVector<double>>& hypervolumes,
                                   const QVector<QString>& policyNames);

    /**
     * @brief Construye el prompt completo que se enviará a Gemini
     * Incluye descripción del problema, evolución reciente, mejores soluciones
     * y parámetros actuales del algoritmo
     */
    QString buildGeneticPrompt(const QString& contentScenario,
                              const QString& last10Hypervolumes,
                              const Individual& kneePoint,
                              const Individual& bestMakespan,
                              const Individual& bestEnergy,
                              const QVector<float>& mutationRates,
                              float crossoverRate);

    /**
     * @brief Realiza la llamada real a la API de Google Gemini
     * @param prompt Texto completo del prompt
     * @param apiKey Clave de autenticación
     * @param temperature Nivel de creatividad (0.0 = determinista, 1.0 = más creativo)
     * @return Respuesta generada por el modelo (normalmente sugerencias de parámetros)
     */
    QString callGeminiAPI(const QString &prompt,
                         const QString &apiKey,
                         double temperature = 0.7);
};

#endif // MAINWINDOW_H