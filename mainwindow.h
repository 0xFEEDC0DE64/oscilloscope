#pragma once

// Qt includes
#include <QMainWindow>
#include <QActionGroup>

// system includes
#include <memory>

// forward declares
class QAudioDeviceInfo;
class QLabel;
namespace Ui { class MainWindow; }
class BaseDevice;

class BaseToneGenerator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void start();
    void stop();
    void refreshRateChanged();
    void zoomChanged();
    void startGenerator();

private:
    int samplerate() const;

    const std::unique_ptr<Ui::MainWindow> m_ui;

    const QList<QAudioDeviceInfo> m_inputDevices, m_outputDevices;

    std::unique_ptr<BaseDevice> m_input;

    QActionGroup m_inputDeviceGroup{this}, m_outputDeviceGroup{this}, m_samplerateGroup{this}, m_refreshrateGroup{this}, m_zoomlevelsGroup{this};

    QLabel &m_statusLabel;

    std::unique_ptr<BaseToneGenerator> m_generator;
};
