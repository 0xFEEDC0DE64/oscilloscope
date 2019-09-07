#include "basetonegenerator.h"

// Qt includes
#include <QAudioOutput>

namespace
{
//! private helper to allow QAudioOutput to read from a io device
class BaseToneGeneratorHelper : public QIODevice
{
public:
    BaseToneGeneratorHelper(BaseToneGenerator &generator, QObject *parent = nullptr);

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    BaseToneGenerator &m_generator;
};

class BaseToneGeneratorPrivate
{
public:
    BaseToneGeneratorPrivate(BaseToneGenerator &generator, const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format);
    ~BaseToneGeneratorPrivate() { output.stop(); }

    BaseToneGeneratorHelper helper;
    QAudioOutput output;
};

}

BaseToneGenerator::BaseToneGenerator() = default;

BaseToneGenerator::~BaseToneGenerator() = default;

void BaseToneGenerator::start()
{
    Q_ASSERT(!running());

    QAudioFormat format;
    format.setSampleRate(m_samplerate);
    format.setChannelCount(2);
    format.setSampleSize(sizeof(SamplePair::Type) * 8);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);

    m_private = std::make_unique<BaseToneGeneratorPrivate>(*this, m_device, format);
    m_private->output.start(&m_private->helper);

}

void BaseToneGenerator::stop()
{
    Q_ASSERT(running());

    m_private = nullptr;
}

namespace {
BaseToneGeneratorHelper::BaseToneGeneratorHelper(BaseToneGenerator &generator, QObject *parent) :
    QIODevice{parent}, m_generator{generator}
{
    setOpenMode(QIODevice::ReadOnly);
}

qint64 BaseToneGeneratorHelper::readData(char *data, qint64 maxlen)
{
    Q_ASSERT(maxlen % sizeof(SamplePair) == 0);

    return m_generator.fill(reinterpret_cast<SamplePair*>(data),
                            reinterpret_cast<SamplePair*>(data) + (maxlen/sizeof(SamplePair))) * sizeof(SamplePair);
}

qint64 BaseToneGeneratorHelper::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)
    qFatal("writing is not allowed!");
}

BaseToneGeneratorPrivate::BaseToneGeneratorPrivate(BaseToneGenerator &generator, const QAudioDeviceInfo &audioDeviceInfo, const QAudioFormat &format) :
    helper{generator}, output{audioDeviceInfo, format}
{
}
}
