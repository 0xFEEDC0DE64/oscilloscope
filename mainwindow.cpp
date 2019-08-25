#include "mainwindow.h"

#include <QAudioInput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QButtonGroup>

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

    enum { Button50, Button100, Button200, Button400 };

    auto buttonGroup = new QButtonGroup;
    buttonGroup->setExclusive(true);
    buttonGroup->addButton(m_ui.radioButtonScale50, Button50);
    buttonGroup->addButton(m_ui.radioButtonScale100, Button100);
    buttonGroup->addButton(m_ui.radioButtonScale200, Button200);
    buttonGroup->addButton(m_ui.radioButtonScale400, Button400);

    const auto factor = m_ui.widget->factor();
    if (factor == .5f) buttonGroup->button(Button50)->click();
    else if (factor == 1.f) buttonGroup->button(Button100)->click();
    else if (factor == 2.f) buttonGroup->button(Button200)->click();
    else if (factor == 4.f) buttonGroup->button(Button400)->click();

    connect(buttonGroup, qOverload<int>(&QButtonGroup::buttonClicked), this, [&widget=*m_ui.widget](int index){
        float factor;

        switch (index)
        {
        case Button50: factor = .5f; break;
        case Button100: factor = 1.f; break;
        case Button200: factor = 2.f; break;
        case Button400: factor = 4.f; break;
        default: Q_UNREACHABLE();
        }

        widget.setFactor(factor);
    });

    toggle();
}

void MainWindow::toggle()
{
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
