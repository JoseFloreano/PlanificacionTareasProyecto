# Planificación de Tareas con Algoritmo Evolutivo Multiobjetivo

Proyecto final de la materia **Tópicos Avanzados de Algoritmos Bioinspirados**  
**Objetivo:** Implementación de un algoritmo evolutivo multiobjetivo (basado en NSGA-II) con **adaptación dinámica de parámetros** para resolver el problema de **planificación de tareas**  considerando dos objetivos en conflicto: minimizar el **makespan** y minimizar el **consumo energético**.

## Características principales

- Optimización multiobjetivo (Makespan vs. Consumo Energético)
- Soporte para múltiples políticas/codificaciones simultáneas
- Adaptación dinámica de hiperparámetros mediante integración con modelo de lenguaje (Google Gemini)
- Visualización avanzada de resultados:
  - Frente de Pareto por política
  - Población completa
  - Evolución del hipervolumen
  - Evolución de hiperparámetros
  - Diagramas de Gantt interactivos por política
- Múltiples operadores de mutación:
  - Inter-cromosoma
  - Intercambio recíproco
  - Shift
- Políticas de dispatching soportadas (ejemplos):
  - FIFO
  - LTP (Longest Total Processing time)
  - STP (Shortest Total Processing time)
  - RRFIFO, RRLTP, RRECA (variantes Round-Robin)

## Tecnologías utilizadas

- Lenguaje: C++ (Qt 6)
- Framework gráfico: Qt Widgets + Qt Charts
- Algoritmo base: NSGA-II adaptado
- Integración IA: API de Google Gemini (para adaptación inteligente de tasas de cruce y mutación)
- Compilación: CMake (recomendado)

## Estructura del proyecto

PlanificacionTareasProyecto/
├── src/
│   ├── main.cpp
│   ├── MainWindow.*               # Interfaz principal
│   ├── EvolutionaryAlgorithm.*    # Núcleo del algoritmo evolutivo
│   ├── Individual.*               # Individuo (contenedor de cromosomas)
│   ├── Chromosome.*               # Cromosoma por política
│   ├── ChartPlotter.*             # Generación de gráficas
│   ├── ScenarioLoader.*           # Carga de instancias
│   ├── Mutations.*                # Operadores de mutación
│   └── ...
├── include/
│   ├── ScenarioData.h
│   ├── ScenarioTypes.h
│   ├── ScheduleTypes.h
│   └── ...
├── resources/
│   ├── instances/                 # Archivos de prueba (.txt)
│   └── ui/                        # Archivos .ui de Qt Designer
├── CMakeLists.txt
├── README.md
└── LICENSE

## Requisitos

- Qt 6.x (Widgets + Charts)
- Compilador C++17 o superior
- CMake 3.15+
- Conexión a internet (para la adaptación mediante API de Gemini)

## Instalación y compilación

# Clonar el repositorio
git clone https://github.com/tu-usuario/PlanificacionTareasProyecto.git
cd PlanificacionTareasProyecto

# Crear directorio de compilación
mkdir build && cd build

# Configurar con CMake
cmake .. -DCMAKE_PREFIX_PATH=/ruta/a/Qt/6.x.x/gcc_64   # Ajustar ruta según tu instalación

# Compilar
cmake --build . --config Release

# Ejecutar
./PlanificacionTareasProyecto

## Uso básico

1. Abrir la aplicación
2. Cargar una instancia del problema (botón "Cargar Instancia")
3. Seleccionar las políticas a utilizar
4. Configurar parámetros iniciales (opcional)
5. Presionar Iniciar
6. Observar en tiempo real:
   - Evolución del frente de Pareto
   - Hipervolumen por política
   - Evolución de hiperparámetros
   - Mejores soluciones encontradas
   - Diagramas de Gantt detallados

## Resultados esperados

El sistema genera automáticamente:
- Frente de Pareto por cada política
- Soluciones destacadas:
  - Mejor en makespan
  - Mejor en consumo energético
  - Punto de rodilla (knee point) — trade-off recomendado
- Gráficas de evolución de hipervolumen y parámetros
- Programaciones detalladas visualizadas mediante diagramas de Gantt

## Autores

-  Jose Luis Floreano Hernández
  Estudiante de Ingeniería en Inteligencia Artificial
  Tópicos Avanzados de Algoritmos Bioinspirados – 2025
- Ian Pablo Martinez Fuentes  
  Estudiante de Ingeniería en Inteligencia Artificial
  Tópicos Avanzados de Algoritmos Bioinspirados – 2025
- Jesus Enrique Vazquez Martinez  
  Estudiante de Ingeniería en Inteligencia Artificial
  Tópicos Avanzados de Algoritmos Bioinspirados – 2025

## Licencia

[MIT License](LICENSE)  
Puedes usar, modificar y distribuir este código siempre que incluyas la licencia original.