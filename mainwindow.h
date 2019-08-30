#pragma once

// Qt includes
#include <QMainWindow>

// system includes
#include <memory>

// forward declares
class QAudioDeviceInfo;
namespace Ui { class MainWindow; }
class BaseDevice;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void toggle();

private:
    const std::unique_ptr<Ui::MainWindow> m_ui;

    const QList<QAudioDeviceInfo> m_audioDevices;

    const std::unique_ptr<BaseDevice> m_input;
};
