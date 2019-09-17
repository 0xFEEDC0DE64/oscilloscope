#pragma once

// Qt includes
#include <QOpenGLWidget>
#include <QPixmap>
#include <QPointF>
#include <QElapsedTimer>

// system includes
#include <vector>

// local includes
#include "oscicommon.h"

class OsciWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit OsciWidget(QWidget *parent = nullptr);

    float factor() const { return m_factor; }
    int fps() const { return m_fps; }
    float afterglow() const { return m_afterglow; }
    int lightspeed() const;

signals:
    void statusUpdate(const QString &status);

public slots:
    void setFactor(float factor) { m_factor = factor; }
    void setFps(int fps);
    void setAfterglow(float afterglow);
    void setLightspeed(int lightspeed);

    void renderSamples(const SamplePair *begin, const SamplePair *end);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void updateFrameBuffer();

private:
    float m_factor{2.f};
    int m_fps{30}, m_afterglowColor{175};
    float m_afterglow{0.2};
    float m_lightspeed{35.f};

    std::vector<SamplePair> m_buffer;

    int m_frameCounter{0}, m_callbacksCounter{0};
    QElapsedTimer m_statsTimer;

    int m_redrawTimerId;
    QPointF m_lastPoint;
    QPixmap m_pixmap;
};
