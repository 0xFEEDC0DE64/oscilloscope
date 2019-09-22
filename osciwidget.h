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
    float afterglow() const { return m_decayTime; }
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
    void updateDrawBuffer();

private:
    float m_factor{1.f};
    int m_fps{60};
    float m_decayTime{25.0};
    float m_lightspeed{35.f};

    typedef std::vector<SamplePair> SampleBuffer;
    SampleBuffer m_buffer;
    SampleBuffer::iterator m_bufferOffset;

    int m_frameCounter{0}, m_callbacksCounter{0}, m_samplesCounter{0};
    QElapsedTimer m_statsTimer;
    QElapsedTimer m_bufferTimer;
    qint64 m_lastTime;

    int m_redrawTimerId;
    QPointF m_lastPoint;
    QPixmap m_pixmap;
    void darkenFrame();
    void drawBuffer(SampleBuffer::iterator &bufferPos, const SampleBuffer::iterator &end);
    void resizeDrawBuffer();
};
