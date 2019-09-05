#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPointF>
#include <QElapsedTimer>

#include "audiodevice.h"

class OsciWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OsciWidget(QWidget *parent = nullptr);

    int blend() const;
    float factor() const;
    float glow() const;
    int fps() const;

public slots:
    void setBlend(int blend);
    void setFactor(float factor);
    void setGlow(float glow);
    void setFps(int fps);

    void renderSamples(const SamplePair *begin, const SamplePair *end);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void resizePixmap();
    void createBlendPixmap();

    int m_blend{150};
    float m_factor{4.f};
    float m_glow{512.f};

    QPixmap m_pixmap;

    QPointF m_lastPoint;

    int m_frameCounter{0}, m_displayFrameCounter{0};
    QElapsedTimer m_fpsTimer;

    int m_fps{15};
    int m_redrawTimerId;
};
