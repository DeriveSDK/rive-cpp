#ifndef _RIVE_RENDERER_HPP_
#define _RIVE_RENDERER_HPP_

#include "rive/shapes/paint/color.hpp"
#include "rive/command_path.hpp"
#include "rive/layout.hpp"
#include "rive/refcnt.hpp"
#include "rive/math/aabb.hpp"
#include "rive/math/mat2d.hpp"
#include "rive/shapes/paint/blend_mode.hpp"
#include "rive/shapes/paint/stroke_cap.hpp"
#include "rive/shapes/paint/stroke_join.hpp"
#include <cmath>
#include <stdio.h>
#include <cstdint>
#include <vector>

namespace rive {
    class Vec2D;

    using Unichar = int32_t;
    using GlyphID = uint16_t;

    class RenderFont {
    public:
        struct AxisInfo {
            uint32_t    tag;
            float       min;
            float       def;    // default value
            float       max;
        };
        
        virtual int countAxes() const { return 0; }
        virtual std::vector<AxisInfo> getAxes() const { return std::vector<AxisInfo>(); }
        
        // TODO: getGlyphPath(index) -> rawpath
    };

    struct RenderTextRun {
        rcp<RenderFont> font;
        float           size;
        uint32_t        textCount;  // number of unichars in this run in text[]
    };

    struct RenderGlyphRun {
        rcp<RenderFont>         font;
        float                   size;

        uint32_t                startTextIndex;
        std::vector<GlyphID>    glyphs;
        std::vector<float>      xpos;   // xpos.size() == glyphs.size() + 1
    };

    extern std::vector<RenderGlyphRun> shapeText(const Unichar text[], size_t textCount,
                                                 const RenderTextRun[], size_t runCount);

    enum class RenderPaintStyle { stroke, fill };

    class RenderPaint {
    public:
        virtual void style(RenderPaintStyle style) = 0;
        virtual void color(ColorInt value) = 0;
        virtual void thickness(float value) = 0;
        virtual void join(StrokeJoin value) = 0;
        virtual void cap(StrokeCap value) = 0;
        virtual void blendMode(BlendMode value) = 0;

        virtual void linearGradient(float sx, float sy, float ex, float ey) = 0;
        virtual void radialGradient(float sx, float sy, float ex, float ey) = 0;
        virtual void addStop(ColorInt color, float stop) = 0;
        virtual void completeGradient() = 0;
        virtual ~RenderPaint() {}
    };

    class RenderImage {
    protected:
        int m_Width = 0;
        int m_Height = 0;

    public:
        virtual ~RenderImage() {}
        virtual bool decode(const uint8_t* bytes, std::size_t size) = 0;
        int width() const { return m_Width; }
        int height() const { return m_Height; }
    };

    class RenderPath : public CommandPath {
    public:
        RenderPath* renderPath() override { return this; }
        void addPath(CommandPath* path, const Mat2D& transform) override {
            addRenderPath(path->renderPath(), transform);
        }

        virtual void addRenderPath(RenderPath* path,
                                   const Mat2D& transform) = 0;
    };

    class Renderer {
    public:
        virtual ~Renderer() {}
        virtual void save() = 0;
        virtual void restore() = 0;
        virtual void transform(const Mat2D& transform) = 0;
        virtual void drawPath(RenderPath* path, RenderPaint* paint) = 0;
        virtual void clipPath(RenderPath* path) = 0;
        virtual void
        drawImage(RenderImage* image, BlendMode value, float opacity) = 0;

        // helpers

        void translate(float x, float y);
        void scale(float sx, float sy);
        void rotate(float radians);

        void computeAlignment(Mat2D& result,
                              Fit fit,
                              const Alignment& alignment,
                              const AABB& frame,
                              const AABB& content);

        void align(Fit fit,
                   const Alignment& alignment,
                   const AABB& frame,
                   const AABB& content);
    };

    extern RenderPath* makeRenderPath();
    extern RenderPaint* makeRenderPaint();
    extern RenderImage* makeRenderImage();
} // namespace rive
#endif
