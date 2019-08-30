#include "osciwidget.h"

#include <cmath>

#include <QLine>

namespace {
template<typename T>
auto pythagoras(const T &dx, const T &dy)
{
    return std::sqrt(dx * dx + dy * dy);
}

auto pythagoras(const QLine &line)
{
    return pythagoras(line.dx(), line.dy());
}
}

OsciWidget::OsciWidget(QWidget *parent) :
    QWidget(parent)
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

    for (auto i = begin; i < end; i++)
    {
        const qint32 x = (float(i->x) / std::numeric_limits<qint16>::max() / 2 * width() / 2 * m_factor) + (width() / 2);
        const qint32 y = (float(-i->y) / std::numeric_limits<qint16>::max() / 2 * height() / 2 * m_factor) + (height() / 2);
        const QPoint p{x,y};

        if (Q_LIKELY(m_lastPoint.has_value()))
        {
            auto dist = pythagoras(QLine(*m_lastPoint, p));
            if (dist < 1)
                dist = 1;

            pen.setColor(QColor(0, 1./dist*255, 0));
            painter.setPen(pen);

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

void OsciWidget::restartTimer()
{
    if (m_timerId != -1)
        killTimer(m_timerId);

    m_timerId = startTimer(1000/m_framerate);
}

{
}

void OsciWidget::resizePixmap()
{
    m_pixmap = QPixmap(size());
    m_pixmap.fill(QColor());
}
