#pragma once

#include <SM_Vector.h>
#include <unirender/typedef.h>

#include <memory>

namespace ur { class Device; }

namespace tess
{

class Palette
{
public:
	Palette(const ur::Device& dev);

    auto GetTexture() const { return m_tex; }

	static const sm::vec2 UV_RED;
	static const sm::vec2 UV_GREEN;
	static const sm::vec2 UV_BLUE;
	static const sm::vec2 UV_WHITE;

private:
    ur::TexturePtr m_tex = nullptr;

}; // Palette

}