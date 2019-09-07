#include "debugtonegenerator.h"

// system includes
#include <cmath>
#include <limits>

std::size_t DebugToneGenerator::fill(SamplePair *begin, SamplePair *end)
{
    for (auto iter = begin; iter != end; iter++)
    {
        Q_ASSERT(iter <= end);
        iter->first = std::sin(m_counter) * std::numeric_limits<SamplePair::Type>::max() / 2;
        iter->second = std::sin((m_counter*8.f) + std::sin(m_offset)*10.) *(0.5 * (.7f + (std::sin(m_counter - M_PI/2) * .3f))) * std::numeric_limits<SamplePair::Type>::max() / 2;

        m_counter+= 1. / samplerate() * (1000. + (std::sin(m_freq) * 500.));
        m_offset+=0.00001f;
        m_freq+=1. / samplerate() * 4;
    }

    while (m_counter >= M_PI * 2)
        m_counter -= M_PI * 2;

    return std::distance(begin, end);
}
