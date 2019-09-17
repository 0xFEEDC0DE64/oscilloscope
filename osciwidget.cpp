#include "osciwidget.h"

// Qt includes
#include <QLineF>
#include <QDebug>
#include <QPainter>
#include <QTimerEvent>

// system includes
#include <cmath>

OsciWidget::OsciWidget(QWidget *parent) :
    QOpenGLWidget{parent},
    m_redrawTimerId(startTimer(1000/m_fps))
{
    m_statsTimer.start();
}

int OsciWidget::lightspeed() const
{
    return std::cbrt(m_lightspeed) * 20.f;
}

void OsciWidget::setFps(int fps)
{
    killTimer(m_redrawTimerId);

    m_fps = fps;

    m_redrawTimerId = startTimer(1000/m_fps);
}

void OsciWidget::setAfterglow(float afterglow){
    m_afterglow = afterglow;
    // percentage of the image that should be visible after one second
    // i.e. factor^fps=afterglow -> factor = afterglow^(1/fps)
}

void OsciWidget::setLightspeed(int lightspeed) {
    const auto temp = (float(lightspeed)/20.f);
    m_lightspeed = temp*temp*temp;
    qDebug() << m_lightspeed;
}

void OsciWidget::renderSamples(const SamplePair *begin, const SamplePair *end)
{
    m_callbacksCounter++;

    m_samplesCounter += std::distance(begin, end);

    m_buffer.insert(m_buffer.end(), begin, end);
}

void OsciWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    m_frameCounter++;
    if (m_statsTimer.hasExpired(1000))
    {
        emit statusUpdate(QString("%0FPS (%1 callbacks, %2 samples, %3 avg per callback)").arg(m_frameCounter).arg(m_callbacksCounter).arg(m_samplesCounter).arg(m_callbacksCounter>0?m_samplesCounter/m_callbacksCounter:0));
        m_frameCounter = 0;
        m_callbacksCounter = 0;
        m_samplesCounter = 0;
        m_statsTimer.restart();
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, m_pixmap);
}

void OsciWidget::updateFrameBuffer()
{
    // Workaround for flickering (do not update, when there is no new data)
    if(m_buffer.empty()) return;

    if (m_pixmap.size() != size())
    {
        m_pixmap = QPixmap(size());
        m_pixmap.fill(Qt::black);
    }

    QPainter painter(&m_pixmap);

    // darkening last frame
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setPen({});
    auto afterglowColor = 255 * pow(m_afterglow, 1.0/m_fps);
    painter.setBrush(QColor(afterglowColor, afterglowColor, afterglowColor));
    painter.drawRect(m_pixmap.rect());

    // drawing new lines ontop
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(0, 255, 0));
    painter.setPen(pen);
    painter.translate(m_pixmap.width()/2, m_pixmap.height()/2);
    painter.setCompositionMode(QPainter::CompositionMode_Plus);

    const auto pointToCoordinates = [width=m_pixmap.width()/2,height=m_pixmap.height()/2,factor=m_factor](const QPointF &point)
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

        painter.setOpacity(std::min(1.0, 1. / ((line.length() * m_lightspeed) + 1)));

        painter.drawLine(pointToCoordinates(m_lastPoint), pointToCoordinates(p));

        m_lastPoint = p;
    }

    m_buffer.clear();
}

void OsciWidget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);
    if (event->timerId() == m_redrawTimerId){
        updateFrameBuffer();
        repaint();
    }
}
