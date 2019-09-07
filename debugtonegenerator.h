#include "basetonegenerator.h"

class DebugToneGenerator : public BaseToneGenerator
{
public:
    using BaseToneGenerator::BaseToneGenerator;

    std::size_t fill(SamplePair *begin, SamplePair *end) override;

private:
    float m_counter{0.f};
    float m_offset{0.};
    float m_freq{0.};
};
