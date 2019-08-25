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

    int framerate() const;
    int blend() const;
    float factor() const;

public slots:
    void setFramerate(int framerate);
    void setBlend(int blend);
    void setFactor(float factor);

    void renderSamples(const SamplePair *begin, const SamplePair *end);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void restartTimer();
    void resizePixmap();
    void createBlendPixmap();

    int m_timerId{-1};
    int m_framerate{15};
    int m_blend{190};
    float m_factor{4.f};

    QPixmap m_pixmap;
    QPixmap m_fadeoutPixmap;

    std::optional<QPoint> m_lastPoint;
};
