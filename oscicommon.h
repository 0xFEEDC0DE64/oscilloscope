#pragma once

// Qt includes
#include <QtGlobal>

template<typename T>
struct SamplePairT {
    typedef T Type;
    T first, second;
};

using SamplePair = SamplePairT<qint16>;
