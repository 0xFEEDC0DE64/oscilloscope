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

public slots:
    void setFactor(float factor) { m_factor = factor; }
    void setFps(int fps);

    void renderSamples(const SamplePair *begin, const SamplePair *end);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    float m_factor{2.f};

    QPointF m_lastPoint;

    int m_frameCounter{0}, m_callbacksCounter{0};
    QString m_statsDisplay;
    QElapsedTimer m_fpsTimer;

    int m_fps{15};
    int m_redrawTimerId;

    std::vector<SamplePair> m_buffer;
};
