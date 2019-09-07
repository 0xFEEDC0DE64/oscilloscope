#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt includes
#include <QLabel>
#include <QWidgetAction>
#include <QFormLayout>
#include <QSpinBox>
#include <QDebug>

// local includes
#include "audiodevice.h"
#include "fakedevice.h"

namespace {
constexpr int samplerates[] = { 44100, 48000, 96000, 192000 };

constexpr int refreshrates[] = { 15, 30, 50, 60 };

constexpr int zoomlevels[] = { 50, 75, 100, 200, 400, 800 };

template<typename T>
void setActionsEnabled(const T &actions, bool enabled)
{
    for(auto action : actions)
        action->setEnabled(enabled);
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow{parent},
    m_ui{std::make_unique<Ui::MainWindow>()},
    m_audioDevices{QAudioDeviceInfo::availableDevices(QAudio::AudioInput)},
    m_statusLabel{*new QLabel}
{
    m_ui->setupUi(this);

    m_ui->statusbar->addWidget(&m_statusLabel);

    connect(m_ui->widget, &OsciWidget::statusUpdate, &m_statusLabel, &QLabel::setText);

    // setting up menu File
    connect(m_ui->actionStart, &QAction::triggered, this, &MainWindow::start);
    connect(m_ui->actionStop, &QAction::triggered, this, &MainWindow::stop);
    m_ui->actionQuit->setShortcut(QKeySequence::Quit);

    // setting up menu Devices
    for (const auto &device : m_audioDevices)
    {
        auto name = device.deviceName();
        const auto action = m_ui->menuDevice->addAction(name);
        action->setCheckable(true);
        m_deviceGroup.addAction(action);

        // Select last element containing monitor if available
        if(name.contains("monitor"))
            action->setChecked(true);
    }

    // setting up menu Samplerates
    for (const auto samplerate : samplerates)
    {
        auto action = m_ui->menuSamplerate->addAction(tr("%0").arg(samplerate));
        action->setCheckable(true);
        m_samplerateGroup.addAction(action);
    }

    m_samplerateGroup.actions().first()->setChecked(true);

    // setting up menu Refreshrates
    for (const auto refreshrate : refreshrates)
    {
        auto action = m_ui->menuRefreshrate->addAction(tr("%0FPS").arg(refreshrate));
        action->setCheckable(true);
        m_refreshrateGroup.addAction(action);
    }

    {
        const auto index = std::find(std::begin(refreshrates), std::end(refreshrates), m_ui->widget->fps());
        if (index != std::end(refreshrates))
            m_refreshrateGroup.actions().at(std::distance(std::begin(refreshrates), index))->setChecked(true);
    }

    connect(&m_refreshrateGroup, &QActionGroup::triggered, this, &MainWindow::refreshRateChanged);

    // setting up menu Zoom
    for (const auto zoomlevel : zoomlevels)
    {
        auto action = m_ui->menuZoom->addAction(tr("%0%").arg(zoomlevel));
        action->setCheckable(true);
        m_zoomlevelsGroup.addAction(action);
    }

    {
        const auto index = std::find(std::begin(zoomlevels), std::end(zoomlevels), m_ui->widget->factor()*100);
        if (index != std::end(zoomlevels))
            m_zoomlevelsGroup.actions().at(std::distance(std::begin(zoomlevels), index))->setChecked(true);
    }

    connect(&m_zoomlevelsGroup, &QActionGroup::triggered, this, &MainWindow::zoomChanged);

    //setting up menu Debug
    {
        auto widgetAction = new QWidgetAction(this);
        auto widget = new QWidget;
        auto layout = new QFormLayout(widget);
        {
            auto input = new QSpinBox;
            input->setRange(0, 255);
            input->setValue(m_ui->widget->afterglow());
            connect(input, qOverload<int>(&QSpinBox::valueChanged), m_ui->widget, &OsciWidget::setAfterglow);
            layout->addRow(tr("Afterglow:"), input);
        }
        {
            auto input = new QSpinBox;
            input->setRange(0, 255);
            input->setValue(m_ui->widget->lightspeed());
            connect(input, qOverload<int>(&QSpinBox::valueChanged), m_ui->widget, &OsciWidget::setLightspeed);
            layout->addRow(tr("Lightspeed:"), input);
        }
        widgetAction->setDefaultWidget(widget);
        m_ui->menuDebug->addAction(widgetAction);
    }

    // autostart
    if (m_audioDevices.isEmpty())
    {
        m_ui->actionStart->setEnabled(false);
        m_ui->actionStop->setEnabled(false);
    }
    else
        start();
}

void MainWindow::start()
{
    m_input = std::make_unique<AudioDevice>();
    //m_input = std::make_unique<FakeDevice>();

    {
        auto *checked = m_samplerateGroup.checkedAction();
        const auto index = m_samplerateGroup.actions().indexOf(checked);
        const auto samplerate = samplerates[index];
        qDebug() << "samplerate: checked =" << checked << "index =" << index << "value =" << samplerate;
        m_input->setSamplerate(samplerate);
    }

    connect(m_input.get(), &BaseDevice::samplesReceived, m_ui->widget, &OsciWidget::renderSamples);

    if (auto audioDevice = dynamic_cast<AudioDevice*>(m_input.get()))
    {
        const auto device = m_audioDevices.at(m_deviceGroup.actions().indexOf(m_deviceGroup.checkedAction()));
        qDebug() << "setDevice" << device.deviceName();
        audioDevice->setDevice(device);
    }
    m_input->start();

    setActionsEnabled(m_deviceGroup.actions(), false);
    setActionsEnabled(m_samplerateGroup.actions(), false);
    m_ui->actionStart->setEnabled(false);
    m_ui->actionStop->setEnabled(true);
}

void MainWindow::stop()
{
    m_input = nullptr;
    setActionsEnabled(m_deviceGroup.actions(), true);
    setActionsEnabled(m_samplerateGroup.actions(), true);
    m_ui->actionStart->setEnabled(true);
    m_ui->actionStop->setEnabled(false);
}

void MainWindow::refreshRateChanged()
{
    auto *checked = m_refreshrateGroup.checkedAction();
    const auto index = m_refreshrateGroup.actions().indexOf(checked);
    const auto refreshrate = refreshrates[index];
    qDebug() << "refreshrate: checked =" << checked << "index =" << index << "value =" << refreshrate;
    m_ui->widget->setFps(refreshrate);
}

void MainWindow::zoomChanged()
{
    auto *checked = m_zoomlevelsGroup.checkedAction();
    const auto index = m_zoomlevelsGroup.actions().indexOf(checked);
    const auto zoomlevel = zoomlevels[index];
    qDebug() << "zoomlevel: checked =" << checked << "index =" << index << "value =" << zoomlevel;
    m_ui->widget->setFactor(zoomlevel/100.f);
}

MainWindow::~MainWindow() = default;
