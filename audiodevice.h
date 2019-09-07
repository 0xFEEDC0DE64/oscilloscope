#pragma once

#include "basedevice.h"

// Qt includes
#include <QAudioDeviceInfo>

// system includes
#include <memory>

// forward declares
namespace { class AudioDevicePrivate; }
class QAudioInput;

class AudioDevice : public BaseDevice
{
    Q_OBJECT

public:
    explicit AudioDevice(QObject *parent = nullptr);
    ~AudioDevice() override;

    void start() override;
    void stop() override;
    bool running() const override { return m_private != nullptr; }

    int samplerate() const override { return m_samplerate; }
    void setSamplerate(int samplerate) override { Q_ASSERT(!running()); m_samplerate = samplerate; }

    const auto &device() const { return m_device; }
    void setDevice(const QAudioDeviceInfo &device) { Q_ASSERT(!running()); m_device = device; }

private:
    std::unique_ptr<AudioDevicePrivate> m_private;

    int m_samplerate;

    QAudioDeviceInfo m_device;
};
