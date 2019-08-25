#include <QApplication>
#include <QAudioInput>

#include "device.h"
#include "osciwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const auto info = []() -> const QAudioDeviceInfo {
        for (const auto &info : QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
            if (info.deviceName() == "alsa_output.pci-0000_00_1b.0.analog-stereo.monitor")
                return info;
        Q_UNREACHABLE();
    }();

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);

    QAudioInput input(info, format);
    input.setBufferSize(44100/60*4);

    OsciWidget widget;
    widget.show();

    Device device;
    input.start(&device);
    QObject::connect(&device, &Device::samplesReceived, &widget, &OsciWidget::samplesReceived);

    //FakeDevice fake;
    //QObject::connect(&fake, &FakeDevice::samplesReceived, &widget, &OsciWidget::samplesReceived);

    return a.exec();
}
