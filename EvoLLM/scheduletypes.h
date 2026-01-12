#ifndef SCHEDULE_TYPES_H
#define SCHEDULE_TYPES_H

// Estado de máquina
struct MachineState {
    double currentTime = 0.0;
    double totalEnergy = 0.0;
    bool isActive = false;
};

// Operación programada
struct OperationSchedule {
    int operationId = -1;
    int jobId = -1;
    int machineId = -1;
    double startTime = 0.0;
    double endTime = 0.0;
    double processingTime = 0.0;
    double energyCost = 0.0;
};

// Estado de trabajo
struct JobState {
    int nextOperationIndex = 0;
    double lastOperationEndTime = 0.0;
};

#endif // SCHEDULE_TYPES_H
