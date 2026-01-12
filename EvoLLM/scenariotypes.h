#ifndef SCENARIO_TYPES_H
#define SCENARIO_TYPES_H

#include <QVector>
#include <QString>
#include <QMap>

// Operation
struct Operation {
    int id = -1;
    Operation() = default;
    explicit Operation(int _id) : id(_id) {}
};

// Job
struct Job {
    int id = -1;
    QVector<int> operations;

    Job() = default;
    explicit Job(int _id) : id(_id) {}

    void addOperation(int opId) {
        operations.append(opId);
    }
};

#endif // SCENARIO_TYPES_H
