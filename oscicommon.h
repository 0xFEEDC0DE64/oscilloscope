#pragma once

// Qt includes
#include <QtGlobal>

template<typename T>
struct SamplePairT {
    T first, second;
};

using SamplePair = SamplePairT<qint16>;
