#pragma once

#include <QMainWindow>

#include <memory>

#include "device.h"
#include "ui_mainwindow.h"

class QAudioInput;
class QAudioFormat;
class QAudioDeviceInfo;
namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void toggle();

private:
    Ui::MainWindow m_ui;

    std::unique_ptr<QAudioInput> m_input;

    Device m_device;
    //FakeDevice m_fakeDevice;

    const QList<QAudioDeviceInfo> m_audioDevices;
};
