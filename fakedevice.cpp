#include "fakedevice.h"

// Qt includes
#include <QTimerEvent>

// system includes
#include <cmath>

void FakeDevice::start()
{
    Q_ASSERT(!running());
    m_bufferSize = m_samplerate/m_framerate;
    m_buffer = std::make_unique<SamplePair[]>(m_bufferSize);
    m_timerId = startTimer(1000/m_framerate);
}

void FakeDevice::stop()
{
    Q_ASSERT(running());
    killTimer(m_timerId);
    m_buffer = nullptr;
}

bool FakeDevice::running() const
{
    return m_timerId != -1;
}

void FakeDevice::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        for (SamplePair *pair = m_buffer.get();
             pair != m_buffer.get() + m_bufferSize;
             pair++)
        {
            pair->x = std::sin(m_dingsDesHaltHochZaehlt) * std::numeric_limits<qint16>::max();
            pair->y = std::cos(m_dingsDesHaltHochZaehlt) * std::numeric_limits<qint16>::max();
            m_dingsDesHaltHochZaehlt += 0.05;
        }

        emit samplesReceived(m_buffer.get(), m_buffer.get() + m_bufferSize);
    }
    else
        QObject::timerEvent(event);
}
