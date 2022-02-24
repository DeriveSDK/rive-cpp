#ifndef _RIVE_HITTEST_HPP_
#define _RIVE_HITTEST_HPP_

#include "rive/math/aabb.hpp"
#include "rive/math/vec2d.hpp"
#include <cstdint>
#include <vector>

namespace rive {

class HitTester {
    std::vector<uint32_t> m_Atoms;
    Vec2D                 m_First, m_Prev;
    AABB                  m_Clip;
    bool                  m_ExpectsMove;

public:
    HitTester();
    ~HitTester();

    void reset(const AABB&);

    void moveTo(Vec2D);
    void lineTo(Vec2D);
    void quadTo(Vec2D, Vec2D);
    void cubicTo(Vec2D, Vec2D, Vec2D);
    void close();
    
    bool test();
};

}

#endif

