#include "device.h"

#include <cmath>

#include <QTimerEvent>

Device::Device(QObject *parent) :
    QIODevice(parent)
{
    setOpenMode(QIODevice::WriteOnly);
}

qint64 Device::readData(char *data, qint64 maxlen)
{
    qFatal("oida");
}

qint64 Device::writeData(const char *data, qint64 len)
{
    Q_ASSERT(len % sizeof(SamplePair) == 0);
    emit samplesReceived(reinterpret_cast<const SamplePair*>(data),
                         reinterpret_cast<const SamplePair*>(data + (len/sizeof(SamplePair))));
    return len;
}

FakeDevice::FakeDevice(QObject *parent) :
    QObject(parent), m_timerId(startTimer(1000/60))
{
}

void FakeDevice::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        std::array<SamplePair, 44100/60> samples;
        for (SamplePair &pair : samples)
        {
            pair.x = std::sin(m_dingsDesHaltHochZaehlt) * std::numeric_limits<qint16>::max();
            pair.y = std::cos(m_dingsDesHaltHochZaehlt) * std::numeric_limits<qint16>::max();
            m_dingsDesHaltHochZaehlt += 0.05;
        }

        emit samplesReceived(samples.begin(), samples.end());
    }
    else
        QObject::timerEvent(event);
}
