#include "mainwindow.h"
#include "ui_mainwindow.h"

// local includes
#include "audiodevice.h"
#include "debugtonegenerator.h"

// Qt includes
#include <QLabel>
#include <QWidgetAction>
#include <QFormLayout>
#include <QSpinBox>
#include <QDebug>

// system includes
#include <stdexcept>

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
    m_inputDevices{QAudioDeviceInfo::availableDevices(QAudio::AudioInput)},
    m_outputDevices{QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)},
    m_statusLabel{*new QLabel}
{
    m_ui->setupUi(this);

    m_ui->statusbar->addWidget(&m_statusLabel);

    connect(m_ui->widget, &OsciWidget::statusUpdate, &m_statusLabel, &QLabel::setText);

    // setting up menu File
    connect(m_ui->actionStart, &QAction::triggered, this, &MainWindow::start);
    connect(m_ui->actionStop, &QAction::triggered, this, &MainWindow::stop);
    m_ui->actionQuit->setShortcut(QKeySequence::Quit);

    // setting up menu InputDevices
    for (const auto &device : m_inputDevices)
    {
        auto name = device.deviceName();
        const auto action = m_ui->menuInputDevice->addAction(name);
        action->setCheckable(true);
        m_inputDeviceGroup.addAction(action);

        // Select last element containing monitor if available
        if(name.contains("monitor"))
            action->setChecked(true);
    }

    // setting up menu OutputDevices
    for (const auto &device : m_outputDevices)
    {
        auto name = device.deviceName();
        const auto action = m_ui->menuOutputDevice->addAction(name);
        action->setCheckable(true);
        m_outputDeviceGroup.addAction(action);

        // Select last element containing analog-stereo if available
        if(name.contains("analog-stereo"))
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
    connect(m_ui->actionToneGenerator, &QAction::triggered, this, &MainWindow::startGenerator);

    {
        auto widgetAction = new QWidgetAction(this);
        auto widget = new QWidget;
        auto layout = new QFormLayout(widget);
        {
            auto input = new QDoubleSpinBox;
            input->setRange(0, 1.0);
            input->setSingleStep(0.1);
            input->setValue(m_ui->widget->afterglow());
            connect(input, qOverload<double>(&QDoubleSpinBox::valueChanged), m_ui->widget, &OsciWidget::setAfterglow);
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
    if (m_inputDevices.isEmpty())
    {
        m_ui->actionStart->setEnabled(false);
        m_ui->actionStop->setEnabled(false);
    }
    else
        start();
}

void MainWindow::start()
{
    {
        auto input = std::make_unique<AudioDevice>();
        // setDevice is AudioDevice specific API
        input->setDevice(m_inputDevices.at(m_inputDeviceGroup.actions().indexOf(m_inputDeviceGroup.checkedAction())));
        m_input = std::move(input);
    }

    m_input->setSamplerate(samplerate());

    connect(m_input.get(), &BaseDevice::samplesReceived, m_ui->widget, &OsciWidget::renderSamples);

    m_input->start();

    setActionsEnabled(m_inputDeviceGroup.actions(), false);
    setActionsEnabled(m_samplerateGroup.actions(), false);
    m_ui->actionStart->setEnabled(false);
    m_ui->actionStop->setEnabled(true);
}

void MainWindow::stop()
{
    m_input = nullptr;
    setActionsEnabled(m_inputDeviceGroup.actions(), true);
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

void MainWindow::startGenerator()
{
    m_generator = nullptr;
    m_generator = std::make_unique<DebugToneGenerator>();
    m_generator->setDevice(m_outputDevices.at(m_outputDeviceGroup.actions().indexOf(m_outputDeviceGroup.checkedAction())));
    m_generator->setSamplerate(samplerate());
    m_generator->start();
}

int MainWindow::samplerate() const
{
    auto *checked = m_samplerateGroup.checkedAction();
    if (!checked)
        throw std::runtime_error(tr("No samplerate selected!").toStdString());

    const auto index = m_samplerateGroup.actions().indexOf(checked);
    if (index < 0)
        throw std::runtime_error(tr("Unknown samplerate selected!").toStdString());

    if (index >= std::distance(std::begin(samplerates), std::end(samplerates)))
        throw std::runtime_error(tr("Index out of range!").toStdString());

    return samplerates[index];
}

MainWindow::~MainWindow() = default;
