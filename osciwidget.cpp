#include "osciwidget.h"

// Qt includes
#include <QLineF>
#include <QDebug>
#include <QPainter>
#include <QTimerEvent>

// system includes
#include <cmath>

namespace {
    qint32 framesForDuration(qint64 duration){
        return qint32(44100 * duration / 1000000LL);
    }
}


OsciWidget::OsciWidget(QWidget *parent)
    : QOpenGLWidget{parent}
    , m_bufferOffset{m_buffer.begin()}
    , m_lastBufferUpdate{0}
    , m_redrawTimerId(startTimer(1000/m_fps, Qt::PreciseTimer))
{
    m_statsTimer.start();
    m_globalTimer.start();
}

int OsciWidget::lightspeed() const
{
    return std::cbrt(m_lightspeed) * 20.f;
}

void OsciWidget::setFps(int fps)
{
    killTimer(m_redrawTimerId);

    m_fps = fps;

    m_redrawTimerId = startTimer(1000/m_fps, Qt::PreciseTimer);
}

void OsciWidget::setAfterglow(float afterglow)
{
    m_decayTime = afterglow;
    // percentage of the image that should be visible after one second
    // i.e. factor^fps=afterglow -> factor = afterglow^(1/fps)
    // i.e. factor^(fps*persistence)=1/e -> factor = 1/e^(1/(fps*persistence/1000.0))

}

void OsciWidget::setLightspeed(int lightspeed)
{
    const auto temp = (float(lightspeed)/20.f);
    m_lightspeed = temp*temp*temp;
    qDebug() << m_lightspeed;
}

void OsciWidget::renderSamples(const SamplePair *begin, const SamplePair *end)
{
    m_callbacksCounter++;

    m_samplesCounter += std::distance(begin, end);

    auto offset = std::distance(m_buffer.begin(), m_bufferOffset);

    if(m_globalTimer.elapsed()-m_lastBufferUpdate > 5000)
    {
        //qDebug() << "deleting: " << m_bufferOffset - m_buffer.begin() << m_buffer.size();
        // Delete drawn frames
        //m_buffer.erase(m_buffer.begin(), m_bufferOffset);

        // TODO improve by drawing remaining(undrawn) buffer instead
        drawBuffer(m_bufferOffset, m_buffer.end(), QColor(0, 255, 0));
        m_buffer.clear();
        offset = 0;

        m_lastBufferUpdate = m_globalTimer.elapsed();
    }
    //qDebug () << " inserting " << std::distance(begin, end);
    m_buffer.insert(m_buffer.end(), begin, end);
    m_bufferOffset = m_buffer.begin() + offset;
    //m_bufferTimer.restart();
    //qDebug() << m_statsTimer.elapsed();
}

void OsciWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    m_frameCounter++;
    if (m_statsTimer.hasExpired(1000))
    {
        emit statusUpdate(QString("%0FPS (%1 callbacks, %2 samples, %3 avg per callback, bufferSize %4, elapsed %5)").arg(m_frameCounter).arg(m_callbacksCounter).arg(m_samplesCounter).arg(m_callbacksCounter>0?m_samplesCounter/m_callbacksCounter:0).arg(m_buffer.size()).arg(m_globalTimer.elapsed()));
        m_frameCounter = 0;
        m_callbacksCounter = 0;
        m_samplesCounter = 0;
        m_statsTimer.restart();
    }

    QPainter painter(this);
    painter.drawPixmap(0, 0, m_pixmap);
}

void OsciWidget::darkenFrame()
{
    QPainter painter(&m_pixmap);


    //auto afterglowColor = 255 * pow(m_decayTime, 1.0/m_fps);
    auto afterglowFactor = pow(exp(-1), 1000.0/m_decayTime/m_fps);
    auto afterglowColor = static_cast<int>(255*afterglowFactor);
    //qDebug() << afterglowFactor;
    //QColor blendColor;
    //black.setAlpha(afterglowColor);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setBrush(QColor(afterglowColor, afterglowColor, afterglowColor));
    painter.drawRect(m_pixmap.rect());
    //m_pixmap.fill(Qt::black);
}


void OsciWidget::drawBuffer(SampleBuffer::iterator &bufferPos, const SampleBuffer::iterator &end, QColor color)
{
    QPainter painter(&m_pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(m_pixmap.width()/2, m_pixmap.height()/2);
    painter.scale(m_factor * m_pixmap.width() / 2.0, m_factor * m_pixmap.height() / 2.0);

    QPen pen;
    pen.setCosmetic(true); // let pen be scale invariant
    pen.setWidth(2);
    // This prevents most of the overlapping when drawing lines from and to the same points
    pen.setCapStyle(Qt::FlatCap);

    for (;bufferPos < end; ++bufferPos)
    {
        const auto &frame = *bufferPos;

        const QPointF p{
            float(frame.first) / std::numeric_limits<qint16>::max(),
            float(-frame.second) / std::numeric_limits<qint16>::max()
        };

        const QLineF line(m_lastPoint, p);

        auto beamBrightness = std::min(1.0, 1. / ((line.length() * m_lightspeed) + 1));

        // time from bufferPos to end in ms
        double time = 1000.0 * std::distance(bufferPos, end) / 44100.0;

        auto beamDecay = exp(-time/m_decayTime);
        // HACK: gamma rolloff the brightness to make it look better (make configurable?)
        auto beamVisibility = pow(beamBrightness, 2.0)*beamDecay;

        QColor drawColor(color);
        drawColor.setAlphaF(beamVisibility);
        //QColor drawColor(color.red()*beamVisibility, color.green()*beamVisibility, color.blue()*beamVisibility);

        pen.setColor(drawColor);
        painter.setPen(pen);
        //painter.setOpacity(beamVisibility);
        painter.drawLine(m_lastPoint, p);
        //painter.drawLine(m_lastPoint, m_lastPoint);
        //painter.setPen(pen);
        //painter.drawPoint(m_lastPoint);

        m_lastPoint = p;
    }
}

void OsciWidget::resizeDrawBuffer()
{
    if (m_pixmap.size() != size())
    {
        m_pixmap = QPixmap(size());
        m_pixmap.fill(Qt::black);
    }
}

void OsciWidget::updateDrawBuffer()
{
    // If there is no new data do not update
    if(m_buffer.empty()) return;

    resizeDrawBuffer();
    darkenFrame();
    // decay time is the time it needs to decay to 1/e ~ 36,7%

    auto duration = 1000*(m_globalTimer.elapsed()-m_lastBufferUpdate);
    size_t framesOffset = framesForDuration(duration);
    //qDebug() << framesOffset << m_buffer.size()-framesOffset << m_bufferOffset - m_buffer.begin();

    auto bufferEnd = m_buffer.begin() + std::min(framesOffset, m_buffer.size());
    drawBuffer(m_bufferOffset, bufferEnd, QColor(0, 255, 0));
}

void OsciWidget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);
    if (event->timerId() == m_redrawTimerId){
        updateDrawBuffer();
        repaint();
    }
}
