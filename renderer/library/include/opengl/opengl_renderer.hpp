#ifndef _RIVE_OPENGL_RENDERER_HPP_
#define _RIVE_OPENGL_RENDERER_HPP_

#include "low_level/low_level_renderer.hpp"
#include "opengl.h"

namespace rive
{
	class OpenGLRenderer : public LowLevelRenderer
	{
	public:
		OpenGLRenderer();
		~OpenGLRenderer();
		void save() override;
		void restore() override;
		void transform(const Mat2D& transform) override;
		void drawPath(RenderPath* path, RenderPaint* paint) override;
		void clipPath(RenderPath* path) override;

		void onViewportSizeChanged(ViewportSize from, ViewportSize to) override;
		void clear() override;
		void frame() override;
	};

} // namespace rive
#endif