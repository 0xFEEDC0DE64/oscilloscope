#include "audiodevice.h"

// Qt includes
#include <QIODevice>
#include <QAudioInput>

namespace {
//! private helper to allow QAudioInput to write to a io device
class AudioDeviceHelper : public QIODevice
{
public:
    explicit AudioDeviceHelper(AudioDevice &audioDevice);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    AudioDevice &m_audioDevice;
};
}

AudioDevice::AudioDevice(QObject *parent) :
    BaseDevice{parent},
    m_helper(std::make_unique<AudioDeviceHelper>(*this))
{
}

AudioDevice::~AudioDevice() = default;

void AudioDevice::start()
{
    QAudioFormat format;
    format.setSampleRate(m_samplerate);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);

    m_input = std::make_unique<QAudioInput>(m_device, format);
    m_input->start(m_helper.get());
    m_input->setBufferSize(m_samplerate/m_framerate*sizeof(qint16)*2);
}

void AudioDevice::stop()
{
    m_input = nullptr;
}



namespace {
AudioDeviceHelper::AudioDeviceHelper(AudioDevice &audioDevice) :
    QIODevice(&audioDevice),
    m_audioDevice(audioDevice)
{
    setOpenMode(QIODevice::WriteOnly);
}

qint64 AudioDeviceHelper::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)
    qFatal("reading is not allowed!");
}

qint64 AudioDeviceHelper::writeData(const char *data, qint64 len)
{
    Q_ASSERT(len % sizeof(SamplePair) == 0);
    emit m_audioDevice.samplesReceived(reinterpret_cast<const SamplePair*>(data),
                                       reinterpret_cast<const SamplePair*>(data + (len/sizeof(SamplePair))));
    return len;
}
}
