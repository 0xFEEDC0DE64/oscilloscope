#include "audiodevice.h"

// Qt includes
#include <QIODevice>
#include <QAudioInput>
#include <QDebug>

namespace {
//! private helper to allow QAudioInput to write to a io device
class AudioDeviceHelper : public QIODevice
{
public:
    explicit AudioDeviceHelper(AudioDevice &audioDevice);
    ~AudioDeviceHelper();

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    AudioDevice &m_audioDevice;
};

struct AudioDevicePrivate {
    AudioDevicePrivate(AudioDevice &audioDevice, const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format) :
        helper(audioDevice), input(audioDeviceInfo, format)
    {
        qDebug() << audioDeviceInfo.deviceName();
    }

    ~AudioDevicePrivate()
    {
        qDebug() << "called";
    }

    AudioDeviceHelper helper;
    QAudioInput input;
};
}

AudioDevice::AudioDevice(QObject *parent) :
    BaseDevice{parent}
{
}

AudioDevice::~AudioDevice() = default;

void AudioDevice::start()
{
    qDebug() << m_device.deviceName();

    QAudioFormat format;
    format.setSampleRate(m_samplerate);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);

    m_private = std::make_unique<AudioDevicePrivate>(*this, m_device, format);
    m_private->input.start(&m_private->helper);
    //m_private->input.setBufferSize(m_samplerate/m_framerate*sizeof(qint16)*2);
}

void AudioDevice::stop()
{
    qDebug() << "called";
    m_private = nullptr;
}



namespace {
AudioDeviceHelper::AudioDeviceHelper(AudioDevice &audioDevice) :
    QIODevice(&audioDevice),
    m_audioDevice(audioDevice)
{
    qDebug() << "called";
    setOpenMode(QIODevice::WriteOnly);
}

AudioDeviceHelper::~AudioDeviceHelper()
{
    qDebug() << "called";
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
