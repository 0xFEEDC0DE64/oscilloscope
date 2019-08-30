#include "osciwidget.h"

#include <cmath>

#include <QLine>

OsciWidget::OsciWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    restartTimer();
    resizePixmap();
}

int OsciWidget::framerate() const
{
    return m_framerate;
}

int OsciWidget::blend() const
{
    return m_blend;
}

float OsciWidget::factor() const
{
    return m_factor;
}

void OsciWidget::setFramerate(int framerate)
{
    if (framerate == m_framerate)
        return;

    qDebug() << "change framerate to" << framerate;

    m_framerate = framerate;

    restartTimer();
}

void OsciWidget::setBlend(int blend)
{
    if (blend == m_blend)
        return;

    qDebug() << "change blend to" << blend;

    m_blend = blend;
}

void OsciWidget::setFactor(float factor)
{
    m_factor = factor;
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
    painter.translate(width()/2, height()/2);

    for (auto i = begin; i < end; i++)
    {
        const qreal x = (qreal(i->x) / std::numeric_limits<qint16>::max() / 2 * width() / 2 * m_factor);
        const qreal y = (qreal(i->y) / std::numeric_limits<qint16>::max() / 2 * height() / 2 * m_factor);
        const QPointF p{x, y};

        if (Q_LIKELY(m_lastPoint.has_value()))
        {
            auto speed = QLineF(*m_lastPoint, p).length();
            if (speed < 1)
                speed = 1;
            auto brightness = 1./speed;

            painter.setOpacity(brightness);
            painter.drawLine(*m_lastPoint, p);
        }

        m_lastPoint = p;
    }

    painter.end();
}

void OsciWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0, 0, m_pixmap);
    painter.end();
    // Fade pixmap by multiplying all pixels by m_blend

    painter.begin(&m_pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.fillRect(m_pixmap.rect(), QColor(m_blend,m_blend,m_blend));
    painter.end();
}

void OsciWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        repaint();
    }
    else
        QWidget::timerEvent(event);
}

void OsciWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    resizePixmap();
}

void OsciWidget::stop()
{
    if (m_timerId != -1)
        killTimer(m_timerId);
}

void OsciWidget::start()
{
    m_timerId = startTimer(1000/m_framerate);
}

void OsciWidget::restartTimer()
{
    stop();
    start();
}

void OsciWidget::resizePixmap()
{
    m_pixmap = QPixmap(size());
    m_pixmap.fill(QColor());
}
