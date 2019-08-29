#include "mainwindow.h"

#include <QAudioInput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QButtonGroup>
#include <QMessageBox>
#include <QStringBuilder>
#include <QRadioButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_audioDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
{
    m_ui.setupUi(this);

    connect(&m_device, &Device::samplesReceived, m_ui.widget, &OsciWidget::renderSamples);

    //connect(&m_fakeDevice, &FakeDevice::samplesReceived, m_ui.widget, &OsciWidget::samplesReceived);

    for (const auto &device : m_audioDevices)
        m_ui.comboBoxDevices->addItem(device.deviceName());

    for (const auto samplerate : { 44100, 48000, 96000, 192000 })
        m_ui.comboBoxSamplerate->addItem(tr("%0").arg(samplerate), samplerate);

    connect(m_ui.pushButtonToggle, &QAbstractButton::pressed, this, &MainWindow::toggle);

    for (const auto framerate : {15, 30, 50, 60})
        m_ui.comboBoxFps->addItem(tr("%0 FPS").arg(framerate), framerate);

    m_ui.comboBoxFps->setCurrentIndex(m_ui.comboBoxFps->findData(m_ui.widget->framerate()));

    connect(m_ui.comboBoxFps, &QComboBox::currentIndexChanged, this, [&combobox=*m_ui.comboBoxFps,&widget=*m_ui.widget](){
        widget.setFramerate(combobox.currentData().toInt());
    });

    m_ui.spinBoxBlend->setValue(m_ui.widget->blend());

    connect(m_ui.spinBoxBlend, qOverload<int>(&QSpinBox::valueChanged), m_ui.widget, &OsciWidget::setBlend);

    auto buttonGroup = new QButtonGroup;
    buttonGroup->setExclusive(true);    
    for (auto factor : { .5f, 1.f, 2.f, 4.f, 8.f })
    {
        auto radioButton = new QRadioButton(QString::number(factor));
        connect(radioButton, &QRadioButton::pressed, this, [factor,&widget=*m_ui.widget](){
            widget.setFactor(factor);
        });
        m_ui.horizontalLayout->addWidget(radioButton);
    }

    if (m_ui.comboBoxDevices->count())
        toggle();
}

void MainWindow::toggle()
{
    if (!m_ui.comboBoxDevices->count())
    {
        QMessageBox::warning(this, tr("Failed to start!"), tr("Failed to start!") % "\n\n" % tr("No audio devices available!"));
        return;
    }

    if (m_input)
    {
        m_input = nullptr;
        m_ui.comboBoxDevices->setEnabled(true);
        m_ui.comboBoxSamplerate->setEnabled(true);
        m_ui.pushButtonToggle->setText("▶");
    }
    else
    {
        QAudioFormat format;
        format.setSampleRate(m_ui.comboBoxSamplerate->currentData().toInt());
        format.setChannelCount(2);
        format.setSampleSize(16);
        format.setSampleType(QAudioFormat::SignedInt);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);

        m_input = std::make_unique<QAudioInput>(m_audioDevices.at(m_ui.comboBoxDevices->currentIndex()), format);
        m_input->start(&m_device);
        m_input->setBufferSize(format.sampleRate()/60*sizeof(qint16)*2);
        m_ui.comboBoxDevices->setEnabled(false);
        m_ui.comboBoxSamplerate->setEnabled(false);
        m_ui.pushButtonToggle->setText("▮▮");
    }
}

MainWindow::~MainWindow() = default;
