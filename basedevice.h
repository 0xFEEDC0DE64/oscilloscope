#pragma once

// Qt includes
#include <QObject>

// local includes
#include "oscicommon.h"

class BaseDevice : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool running() const = 0;

    virtual int samplerate() const = 0;
    virtual void setSamplerate(int samplerate) = 0;

    void emitSamples(const SamplePair *begin, const SamplePair *end) { emit samplesReceived(begin, end); }

signals:
    void samplesReceived(const SamplePair *begin, const SamplePair *end);
};
