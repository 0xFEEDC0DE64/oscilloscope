#include "audiodevice.h"

// Qt includes
#include <QIODevice>
#include <QAudioInput>

//! private helper to allow QAudioInput to write to a io device
class AudioDeviceHelper : public QIODevice
{
public:
    explicit AudioDeviceHelper(AudioDevice &audioDevice, QObject *parent = nullptr);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    AudioDevice &m_audioDevice;
};

class AudioDevice::AudioDevicePrivate {
public:
    AudioDevicePrivate(AudioDevice &audioDevice, const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format);

    AudioDeviceHelper helper;
    QAudioInput input;
};

AudioDevice::AudioDevice(QObject *parent) :
    BaseDevice{parent}
{
}

AudioDevice::~AudioDevice() = default;

void AudioDevice::start()
{
    Q_ASSERT(!running());

    QAudioFormat format;
    format.setSampleRate(m_samplerate);
    format.setChannelCount(2);
    format.setSampleSize(sizeof(SamplePair::Type) * 8);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);

    m_private = std::unique_ptr<AudioDevicePrivate>(new AudioDevicePrivate(*this, m_device, format));
    m_private->input.start(&m_private->helper);
    //m_private->input.setBufferSize(m_samplerate/m_framerate*sizeof(qint16)*2);
}

void AudioDevice::stop()
{
    Q_ASSERT(running());

    m_private = nullptr;
}

AudioDeviceHelper::AudioDeviceHelper(AudioDevice &audioDevice, QObject *parent) :
    QIODevice{parent}, m_audioDevice(audioDevice)
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

    m_audioDevice.emitSamples(reinterpret_cast<const SamplePair*>(data),
                              reinterpret_cast<const SamplePair*>(data) + (len/sizeof(SamplePair)));

    return len;
}

AudioDevice::AudioDevicePrivate::AudioDevicePrivate(AudioDevice &audioDevice, const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format) :
    helper{audioDevice}, input{audioDeviceInfo, format}
{
}
