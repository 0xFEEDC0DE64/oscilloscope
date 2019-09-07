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

private:
    const std::unique_ptr<Ui::MainWindow> m_ui;

    const QList<QAudioDeviceInfo> m_audioDevices;

    std::unique_ptr<BaseDevice> m_input;

    QActionGroup m_deviceGroup{this}, m_samplerateGroup{this}, m_refreshrateGroup{this}, m_zoomlevelsGroup{this};

    QLabel &m_statusLabel;
};
