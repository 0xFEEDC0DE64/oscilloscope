#include "osciwidget.h"

#include <cmath>

#include <QLine>

namespace {
constexpr auto framerate = 60;
constexpr auto blend = 190;
}

template<typename T>
auto pythagoras(const T &dx, const T &dy)
{
    return std::sqrt(dx * dx + dy * dy);
}

auto pythagoras(const QLine &line)
{
    return pythagoras(line.dx(), line.dy());
}

OsciWidget::OsciWidget(QWidget *parent) :
    QWidget(parent), m_timerId(startTimer(1000/framerate))
{
    resizePixmap();
    m_timer.start();
}

void OsciWidget::samplesReceived(const SamplePair *begin, const SamplePair *end)
{
    QPainter painter;
    painter.begin(&m_pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    QPen pen;
    pen.setWidth(2);

    for (auto i = begin; i < end; i++)
    {
        const qint32 x = (float(i->x) / std::numeric_limits<qint16>::max() / 2 * width() * 2) + (width() / 2);
        const qint32 y = (float(-i->y) / std::numeric_limits<qint16>::max() / 2 * height() * 2) + (height() / 2);
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

    if (m_timer.hasExpired(1000/framerate))
    {
        repaint();
        m_timer.restart();
    }
}

void OsciWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0, 0, m_pixmap);
    painter.end();

    painter.begin(&m_pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.drawPixmap(0, 0, m_fadeoutPixmap);
    painter.end();
}

void OsciWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId)
    {
        //repaint();
    }
    else
        QWidget::timerEvent(event);
}

void OsciWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    resizePixmap();
}

void OsciWidget::resizePixmap()
{
    m_pixmap = QPixmap(size());
    m_pixmap.fill(QColor());

    m_fadeoutPixmap = QPixmap(size());
    m_fadeoutPixmap.fill(QColor(blend, blend, blend));
}
