#include "rive/hittest.hpp"

#include <algorithm>
#include <assert.h>
#include <cmath>

using namespace rive;

struct Point {
    float x, y;
    
    Point(float xx, float yy) : x(xx), y(yy) {}
    Point(const Vec2D& src) : x(src.x()), y(src.y()) {}
};

static inline int graphics_round(float x) {
    return (int)std::floor(x + 0.5f);
}

struct Atom {
    unsigned y : 16;
    unsigned x : 15;
    unsigned w : 1;
};

using Atom32 = uint32_t;

const Atom32 ATOM_SENTINEL = 0xFFFFFFFF;

static int atom_y(Atom32 a) {
    return a >> 16;
}

static int atom_x(Atom32 a) {
    return a << 16 >> 17;
}

static int atom_w(Atom32 a) {
    return ((a & 1) << 1) - 1;
}

static uint32_t pack_atom(int x, int y, int w) {
    assert(x >= 0 && x <= 32767);
    assert(y >= 0 && y <= 65535);
    assert(w == 1 || w == -1);

    uint32_t a = (y << 16) | (x << 1) | ((w + 1) >> 1);

    assert(atom_x(a) == x);
    assert(atom_y(a) == y);
    assert(atom_w(a) == w);
    return a;
}

static bool blit_atoms(const Atom32 atom[]) {
    Atom32 a = *atom++;
    int x = atom_x(a);
    int y = atom_y(a);
    int w = atom_w(a);

    for (;;) {
        if (a == ATOM_SENTINEL) {
            break;
        }
        int left = x;
        const int curr_y = y;
        int curr_w = 0;
        do {
            if (curr_w == 0) {
                left = x;
            }
            curr_w += w;
            if (curr_w == 0 && x > left) {
                return true;    // blit(left, curr_y, x - left)
            }
            a = *atom++;

#ifndef NDEBUG
            int prev_x = x;
#endif
            x = atom_x(a);
            y = atom_y(a);
            w = atom_w(a);

#ifndef NDEBUG
            if (y == curr_y) {
                assert(x >= prev_x);
            }
#endif
        } while (y == curr_y);
    }
    return false;
}

static void append_line(const AABB& clip, Point p0, Point p1, int winding,
                        std::vector<Atom32>& atoms) {
    assert(winding == 1 || winding == -1);

    int top = graphics_round(p0.y);
    int bottom = graphics_round(p1.y);
    if (top == bottom) {
        return;
    }

    if (top > bottom) {
        std::swap(top, bottom);
    }
    assert(top >= clip.top());
    assert(bottom <= clip.bottom());

    const float m = (p1.x - p0.x) / (p1.y - p0.y);
    float x = p0.x + m * (top - p0.y + 0.5f) + 0.5f;

    for (int y = top; y < bottom; ++y) {
        atoms.push_back(pack_atom((int)std::floor(x), y, winding));
        x += m;
    }
}

static void clip_line(const AABB& clip, Point p0, Point p1, std::vector<Atom32>& atoms) {
    if (p0.y == p1.y) {
        return;
    }

    int winding = 1;
    if (p0.y > p1.y) {
        winding = -1;
        std::swap(p0, p1);
    }
    // now we're monotonic in Y: p0 <= p1
    if (p1.y <= clip.top() || p0.y >= clip.bottom()) {
        return;
    }

    double dxdy = (double)(p1.x - p0.x) / (p1.y - p0.y);
    if (p0.y < clip.top()) {
        p0.x += dxdy * (clip.top() - p0.y);
        p0.y = clip.top();
    }
    if (p1.y > clip.bottom()) {
        p1.x += dxdy * (clip.bottom() - p1.y);
        p1.y = clip.bottom();
    }

    // Now p0...p1 is strictly inside clip vertically, so we just need to clip horizontally

    if (p0.x > p1.x) {
        winding = -winding;
        std::swap(p0, p1);
    }
    // now we're left-to-right: p0 .. p1

    if (p1.x <= clip.left()) {   // entirely to the left
        p0.x = p1.x = clip.left();
        append_line(clip, p0, p1, winding, atoms);
    }
    if (p0.x >= clip.right()) {  // entirely to the right
        p0.x = p1.x = clip.right();
        append_line(clip, p0, p1, winding, atoms);
    }

    if (p0.x < clip.left()) {
        float y = p0.y + (clip.left() - p0.x) / dxdy;
        append_line(clip, {clip.left(), p0.y}, {clip.left(), y}, winding, atoms);
        p0 = {clip.left(), y};
    }
    if (p1.x > clip.right()) {
        float y = p0.y + (clip.right() - p0.x) / dxdy;
        append_line(clip, {clip.right(), y}, {clip.right(), p1.y}, winding, atoms);
        p1 = {clip.right(), y};
    }
    append_line(clip, p0, p1, winding, atoms);
}

HitTester::HitTester() {
    this->reset({0, 0, 0, 0});
}

HitTester::~HitTester() {}

void HitTester::reset(const AABB& clip) {
    m_Clip = clip;
    m_Atoms.clear();
    m_ExpectsMove = true;
}

void HitTester::moveTo(Vec2D v) {
    m_First = m_Prev = v;
    m_ExpectsMove = false;
}

void HitTester::lineTo(Vec2D v) {
    assert(!m_ExpectsMove);

    clip_line(m_Clip, m_Prev, v, m_Atoms);
    m_Prev = v;
}

void HitTester::quadTo(Vec2D b, Vec2D c) {
    assert(!m_ExpectsMove);


    m_Prev = c;
}

void HitTester::cubicTo(Vec2D b, Vec2D c, Vec2D d) {
    assert(!m_ExpectsMove);


    m_Prev = d;
}

void HitTester::close() {
    assert(!m_ExpectsMove);

    clip_line(m_Clip, m_Prev, m_First, m_Atoms);
    m_ExpectsMove = true;
}

bool HitTester::test() {
    // todo: auto close last contour

    std::sort(m_Atoms.begin(), m_Atoms.end());
    m_Atoms.push_back(ATOM_SENTINEL);
    bool hit = blit_atoms(m_Atoms.data());

    this->reset({0, 0, 0, 0});
    return hit;
}

