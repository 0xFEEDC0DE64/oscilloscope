#include "osciwidget.h"

#include <cmath>

#include <QLineF>
#include <QDebug>
#include <QPainter>
#include <QTimerEvent>

OsciWidget::OsciWidget(QWidget *parent) :
    QWidget{parent},
    m_redrawTimerId(startTimer(1000/m_fps))
{
    resizePixmap();
    m_fpsTimer.start();
}

int OsciWidget::blend() const
{
    return m_blend;
}

float OsciWidget::factor() const
{
    return m_factor;
}

float OsciWidget::glow() const
{
    return m_glow;
}

int OsciWidget::fps() const
{
    return m_fps;
}

void OsciWidget::setBlend(int blend)
{
    if (blend == m_blend)
        return;

    qDebug() << blend;

    m_blend = blend;
}

void OsciWidget::setFactor(float factor)
{
    qDebug() << factor;
    m_factor = factor;
}

void OsciWidget::setGlow(float glow)
{
    qDebug() << glow;
    m_glow = glow;
}

void OsciWidget::setFps(int fps)
{
    qDebug() << fps;
    killTimer(m_redrawTimerId);
    m_fps = fps;
    m_redrawTimerId = startTimer(1000/m_fps);
}

void OsciWidget::renderSamples(const SamplePair *begin, const SamplePair *end)
{
    QPainter painter;
    painter.begin(&m_pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(0, 255, 0));
    painter.setPen(pen);

    // Paint from center
    painter.translate(m_pixmap.width()/2, m_pixmap.height()/2);

    const auto pointToCoordinates = [width=m_pixmap.width()/2,height=m_pixmap.height()/2,factor=m_factor](const QPointF &point)
    {
        return QPoint{
            int(point.x() * factor * width),
            int(point.y() * factor * height)
        };
    };

    for (auto i = begin; i < end; i++)
    {
        const QPointF p{
            float(i->x) / std::numeric_limits<qint16>::max(),
            float(-i->y) / std::numeric_limits<qint16>::max()
        };

        const QLineF line(m_lastPoint, p);

        auto brightness = 1.f / line.length() / m_glow;
        if (line.length() == 0.f || brightness > 255.f)
            brightness = 255.f;

        painter.setOpacity(brightness);

        painter.drawLine(pointToCoordinates(m_lastPoint), pointToCoordinates(p));

        m_lastPoint = p;
    }

    painter.resetTransform();
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(m_pixmap.rect(), QColor(m_blend,m_blend,m_blend));

    painter.end();
}

void OsciWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    m_frameCounter++;
    if (m_fpsTimer.hasExpired(1000))
    {
        m_displayFrameCounter = m_frameCounter;
        m_frameCounter = 0;
        m_fpsTimer.restart();
    }

    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0, 0, m_pixmap);

    painter.setPen(Qt::white);
    painter.setBrush(Qt::white);
    QFont font;
    font.setPixelSize(24);
    painter.drawText(20, 20, QString("%0FPS").arg(m_displayFrameCounter));

    painter.end();

    if (m_pixmap.rect() != rect())
        qDebug() << m_pixmap.rect() << rect();
}

void OsciWidget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);
    if (event->timerId() == m_redrawTimerId)
        repaint();
}

void OsciWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizePixmap();
}

void OsciWidget::resizePixmap()
{
    m_pixmap = QPixmap(size());
    m_pixmap.fill(QColor{});
}
