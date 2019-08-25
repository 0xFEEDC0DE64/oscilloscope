#pragma once

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QTimerEvent>
#include <QPoint>
#include <QElapsedTimer>

#include <optional>

#include "device.h"

class OsciWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OsciWidget(QWidget *parent = nullptr);

public slots:
    void samplesReceived(const SamplePair *begin, const SamplePair *end);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void resizePixmap();

    const int m_timerId;
    QPixmap m_pixmap;
    QPixmap m_fadeoutPixmap;

    std::optional<QPoint> m_lastPoint;

    QElapsedTimer m_timer;
};
