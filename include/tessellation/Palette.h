#pragma once

#include <SM_Vector.h>

#include <memory>

namespace ur { class RenderContext; class Texture; }

namespace tess
{

class Palette
{
public:
	Palette(ur::RenderContext* rc);

	uint32_t GetTexID() const;

	int GetTexWidth() const;
	int GetTexHeight() const;

	static const sm::vec2 UV_RED;
	static const sm::vec2 UV_GREEN;
	static const sm::vec2 UV_BLUE;
	static const sm::vec2 UV_WHITE;

private:
	std::unique_ptr<ur::Texture> m_tex = nullptr;

}; // Palette

}