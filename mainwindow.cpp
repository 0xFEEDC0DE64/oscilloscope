#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt includes
#include <QButtonGroup>
#include <QMessageBox>
#include <QStringBuilder>
#include <QRadioButton>

// local includes
#include "audiodevice.h"
#include "fakedevice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow{parent},
    m_ui{std::make_unique<Ui::MainWindow>()},
    m_audioDevices{QAudioDeviceInfo::availableDevices(QAudio::AudioInput)},
    m_input{std::make_unique<AudioDevice>()}
    //m_input{std::make_unique<FakeDevice>()}
{
    m_ui->setupUi(this);

    connect(m_input.get(), &BaseDevice::samplesReceived, m_ui->widget, &OsciWidget::renderSamples);

    for (const auto &device : m_audioDevices)
    {
        auto name = device.deviceName();
        m_ui->comboBoxDevices->addItem(name);

        // Select last element containing monitor if available
        if(name.contains("monitor"))
        {
            m_ui->comboBoxDevices->setCurrentIndex(m_ui->comboBoxDevices->count()-1);
        }
    }

    for (const auto samplerate : { 44100, 48000, 96000, 192000 })
        m_ui->comboBoxSamplerate->addItem(tr("%0").arg(samplerate), samplerate);

    connect(m_ui->pushButtonToggle, &QAbstractButton::pressed, this, &MainWindow::toggle);

    for (const auto framerate : {15, 30, 50, 60})
        m_ui->comboBoxFps->addItem(tr("%0 FPS").arg(framerate), framerate);

    connect(m_ui->comboBoxFps, qOverload<int>(&QComboBox::currentIndexChanged), m_ui->widget, [this](){
        m_ui->widget->setFps(m_ui->comboBoxFps->currentData().toInt());
    });

    auto buttonGroup = new QButtonGroup;
    buttonGroup->setExclusive(true);
    for (auto factor : { .5f, 1.f, 2.f, 4.f, 8.f })
    {
        auto radioButton = new QRadioButton(QString::number(factor));
        connect(radioButton, &QRadioButton::pressed, this, [factor,&widget=*m_ui->widget](){
            widget.setFactor(factor);
        });
        m_ui->horizontalLayout->addWidget(radioButton);
    }

    if (m_ui->comboBoxDevices->count())
        toggle();
}

void MainWindow::toggle()
{
    if (!m_ui->comboBoxDevices->count())
    {
        QMessageBox::warning(this, tr("Failed to start!"), tr("Failed to start!") % "\n\n" % tr("No audio devices available!"));
        return;
    }

    if (m_input->running())
    {
        m_input->stop();
        m_ui->comboBoxDevices->setEnabled(true);
        m_ui->comboBoxSamplerate->setEnabled(true);
        m_ui->pushButtonToggle->setText("▶");
    }
    else
    {
        m_input->setSamplerate(m_ui->comboBoxSamplerate->currentData().toInt());
        m_input->setFramerate(60);
        if (auto audioDevice = dynamic_cast<AudioDevice*>(m_input.get()))
            audioDevice->setDevice(m_audioDevices.at(m_ui->comboBoxDevices->currentIndex()));
        m_input->start();

        m_ui->comboBoxDevices->setEnabled(false);
        m_ui->comboBoxSamplerate->setEnabled(false);
        m_ui->pushButtonToggle->setText("▮▮");
    }
}

MainWindow::~MainWindow() = default;
