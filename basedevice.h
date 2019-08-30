#pragma once

#include <QObject>

template<typename T>
struct SamplePairT {
    T x, y;
};

typedef SamplePairT<qint16> SamplePair;

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

    virtual int framerate() const = 0;
    virtual void setFramerate(int framerate) = 0;

signals:
    void samplesReceived(const SamplePair *begin, const SamplePair *end);
};