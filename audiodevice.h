#pragma once

#include "basedevice.h"

// Qt includes
#include <QAudioDeviceInfo>

// system includes
#include <memory>

// forward declares
namespace { class AudioDeviceHelper; }
class QAudioInput;

class AudioDevice : public BaseDevice
{
    Q_OBJECT

public:
    explicit AudioDevice(QObject *parent = nullptr);
    ~AudioDevice() override;

    void start() override;
    void stop() override;
    bool running() const override { return m_input != nullptr; }

    int samplerate() const override { return m_samplerate; }
    void setSamplerate(int samplerate) override { Q_ASSERT(!running()); m_samplerate = samplerate; }

    int framerate() const override { return m_framerate; }
    void setFramerate(int framerate) override { Q_ASSERT(!running()); m_framerate = framerate; }

    const auto &device() const { return m_device; }
    void setDevice(const QAudioDeviceInfo &device) { Q_ASSERT(!running()); m_device = device; }

private:
    const std::unique_ptr<AudioDeviceHelper> m_helper;

    std::unique_ptr<QAudioInput> m_input;

    int m_samplerate;
    int m_framerate;

    QAudioDeviceInfo m_device;
};
