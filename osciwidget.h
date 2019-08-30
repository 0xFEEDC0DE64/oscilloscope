#pragma once

#include <QWidget>

#include <QOpenGLWidget>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QTimerEvent>
#include <QPoint>
#include <QElapsedTimer>

#include <optional>

#include "device.h"

class OsciWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OsciWidget(QWidget *parent = nullptr);

    int framerate() const;
    int blend() const;
    float factor() const;
    float glow() const;
    
    void start();
    void stop();

public slots:
    void setFramerate(int framerate);
    void setBlend(int blend);
    void setFactor(float factor);
    void setGlow(float glow);

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
    int m_blend{150};
    float m_factor{4.f};
    float m_glow{512.f};

    QPixmap m_pixmap;

    std::optional<QPointF> m_lastPoint;
};
