#include "osciwidget.h"

#include <cmath>

#include <QLineF>
#include <QDebug>
#include <QPainter>
#include <QTimerEvent>

OsciWidget::OsciWidget(QWidget *parent) :
    QOpenGLWidget{parent},
    m_redrawTimerId(startTimer(1000/m_fps))
{
    m_fpsTimer.start();
}

void OsciWidget::setFps(int fps)
{
    killTimer(m_redrawTimerId);

    m_fps = fps;

    m_redrawTimerId = startTimer(1000/m_fps);
}

void OsciWidget::renderSamples(const SamplePair *begin, const SamplePair *end)
{
    m_callbacksCounter++;

    m_buffer.insert(m_buffer.end(), begin, end);
}

void OsciWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    m_frameCounter++;
    if (m_fpsTimer.hasExpired(1000))
    {
        m_statsDisplay = QString("%0FPS (%1 callbacks)").arg(m_frameCounter).arg(m_callbacksCounter);
        m_frameCounter = 0;
        m_callbacksCounter = 0;
        m_fpsTimer.restart();
    }

    QPainter painter;
    painter.begin(this);

    // draw background
    painter.setBrush(Qt::black);
    painter.drawRect(rect());

    // drawing new lines ontop
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(0, 255, 0));
    painter.setPen(pen);
    painter.translate(width()/2, height()/2);
    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    const auto pointToCoordinates = [width=width()/2,height=height()/2,factor=m_factor](const QPointF &point)
    {
        return QPoint{
            int(point.x() * factor * width),
            int(point.y() * factor * height)
        };
    };

    for (const auto &i : m_buffer)
    {
        const QPointF p{
            float(i.first) / std::numeric_limits<qint16>::max(),
            float(-i.second) / std::numeric_limits<qint16>::max()
        };

        const QLineF line(m_lastPoint, p);

        painter.setOpacity(std::min(1.0, 1. / ((line.length() * 100) + 1)));

        painter.drawLine(pointToCoordinates(m_lastPoint), pointToCoordinates(p));

        m_lastPoint = p;
    }

    painter.resetTransform();
    painter.setOpacity(1);

    m_buffer.clear();

    // draw stats
    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    QFont font;
    font.setPixelSize(24);
    painter.drawText(20, 20, m_statsDisplay);

    painter.end();
}

void OsciWidget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);
    if (event->timerId() == m_redrawTimerId)
        repaint();
}
