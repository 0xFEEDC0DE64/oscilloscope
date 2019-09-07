#pragma once

#include "basedevice.h"

// system includes
#include <memory>

class FakeDevice : public BaseDevice
{
    Q_OBJECT

public:
    using BaseDevice::BaseDevice;

    void start() override;
    void stop() override;
    bool running() const override;

    int samplerate() const override { return m_samplerate; }
    void setSamplerate(int samplerate) override { Q_ASSERT(!running()); m_samplerate = samplerate; }

    int framerate() const { return m_framerate; }
    void setFramerate(int framerate) { Q_ASSERT(!running()); m_framerate = framerate; }

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    double m_dingsDesHaltHochZaehlt{0.};
    int m_timerId{-1};

    std::unique_ptr<SamplePair[]> m_buffer;
    std::size_t m_bufferSize;

    int m_samplerate{44100};
    int m_framerate{15};
};
