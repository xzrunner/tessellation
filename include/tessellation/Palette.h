#pragma once

#include <SM_Vector.h>
#include <unirender2/typedef.h>

#include <memory>

namespace ur2 { class Device; }

namespace tess
{

class Palette
{
public:
	Palette(const ur2::Device& dev);

    auto GetTexture() const { return m_tex; }

	static const sm::vec2 UV_RED;
	static const sm::vec2 UV_GREEN;
	static const sm::vec2 UV_BLUE;
	static const sm::vec2 UV_WHITE;

private:
    ur2::TexturePtr m_tex = nullptr;

}; // Palette

}