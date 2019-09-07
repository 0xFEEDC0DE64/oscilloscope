#pragma once

// system includes
#include <memory>

// Qt includes
#include <QAudioDeviceInfo>

// local includes
#include "oscicommon.h"

// forward declares
namespace { class BaseToneGeneratorPrivate; }

class BaseToneGenerator
{
public:
    BaseToneGenerator();
    virtual ~BaseToneGenerator();

    void start();
    void stop();
    bool running() const { return static_cast<bool>(m_private); }

    int samplerate() const { return m_samplerate; }
    void setSamplerate(int samplerate) { Q_ASSERT(!running()); m_samplerate = samplerate; }

    const auto &device() const { return m_device; }
    void setDevice(const QAudioDeviceInfo &device) { Q_ASSERT(!running()); m_device = device; }

    virtual std::size_t fill(SamplePair *begin, SamplePair *end) = 0;

private:
    std::unique_ptr<BaseToneGeneratorPrivate> m_private;

    int m_samplerate;

    QAudioDeviceInfo m_device;
};
