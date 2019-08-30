#pragma once

#include <QIODevice>
#include <QDebug>

#define qvoid void

template<typename T> struct SamplePairT {
    T x, y;
};

typedef SamplePairT<qint16> SamplePair;

class Device : public QIODevice
{
    Q_OBJECT

public:
    explicit Device(QObject *parent = nullptr);

signals:
    void samplesReceived(const SamplePair *begin, const SamplePair *end);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
};

class FakeDevice : public QObject
{
    Q_OBJECT

public:
    explicit FakeDevice(QObject *parent = nullptr);

signals:
    void samplesReceived(const SamplePair *begin, const SamplePair *end);

protected:
    qvoid timerEvent(QTimerEvent *event) override;

private:
    double m_dingsDesHaltHochZaehlt;
    const int m_timerId;
};
