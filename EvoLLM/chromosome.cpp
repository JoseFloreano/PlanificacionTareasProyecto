#include "Chromosome.h"

// Constructor por defecto
// Crea un cromosoma vacío con valores iniciales por defecto
Chromosome::Chromosome()
    : policyName(""),           // Nombre de la política (vacío inicialmente)
      f1(0.0),                  // Objetivo 1 (normalmente Makespan)
      f2(0.0),                  // Objetivo 2 (normalmente Consumo de energía)
      domLevel(-1),             // Nivel de dominancia (se calcula después)
      crowdingDistance(-1)      // Distancia de crowding (para NSGA-II, -1 = no calculado)
{
    // No hace nada más, los vectores se inicializan vacíos por defecto
}

// Constructor con nombre de política
// Útil cuando ya se sabe qué política representa este cromosoma
Chromosome::Chromosome(const QString& name)
    : policyName(name),         // Asigna el nombre recibido
      f1(0.0),                  // Objetivos aún sin evaluar
      f2(0.0),
      domLevel(-1),             // Aún no evaluado en términos de dominancia
      crowdingDistance(-1)      // Aún no calculada la distancia de crowding
{
    // Similar al constructor por defecto, solo cambia el nombre
}

// Inicializa el cromosoma con valores aleatorios
// Genera una secuencia aleatoria de enteros que representa la solución codificada
void Chromosome::initializeRandom(int size, int minValue, int maxValue, QRandomGenerator& rng)
{
    genes.clear();              // Asegura que el vector esté vacío antes de empezar
    genes.reserve(size);        // Reserva espacio para mejorar eficiencia (evita realocaciones)

    // Genera 'size' valores enteros aleatorios en el rango [minValue, maxValue]
    for (int i = 0; i < size; ++i) {
        genes.append(rng.bounded(minValue, maxValue + 1));
        // Nota: bounded(a, b) genera [a, b-1] → por eso +1 en el límite superior
    }
}

// Obtiene el tamaño del cromosoma
// Devuelve cuántos genes (valores) contiene actualmente este cromosoma
int Chromosome::size() const
{
    return genes.size();
}

// Convierte el cromosoma a una representación legible como texto
// Útil para depuración, logging o mostrar al usuario
QString Chromosome::toString() const
{
    QString result = policyName + ": [";  // Comienza con el nombre de la política

    // Construye la lista de genes separada por comas
    for (int i = 0; i < genes.size(); ++i) {
        result += QString::number(genes[i]);
        if (i < genes.size() - 1)
            result += ", ";           // coma y espacio entre elementos (excepto el último)
    }

    result += "]";                    // cierra el corchete

    return result;
}