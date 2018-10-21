#include "tessellation/Palette.h"

#include <unirender/Texture.h>

namespace tess
{

// b w
// r g

const sm::vec2 Palette::UV_RED   = sm::vec2(0.25f, 0.25f);
const sm::vec2 Palette::UV_GREEN = sm::vec2(0.75f, 0.25f);
const sm::vec2 Palette::UV_BLUE  = sm::vec2(0.25f, 0.75f);
const sm::vec2 Palette::UV_WHITE = sm::vec2(0.75f, 0.75f);

Palette::Palette(ur::RenderContext* rc)
{
	uint32_t buf[4];
	buf[0] = 0xff0000ff; // r
	buf[1] = 0xff00ff00; // g
	buf[2] = 0xffff0000; // b
	buf[3] = 0xffffffff; // w

	m_tex = std::make_unique<ur::Texture>();
	m_tex->Upload(rc, 2, 2, ur::TEXTURE_RGBA8, reinterpret_cast<unsigned char*>(buf));
}

uint32_t Palette::GetTexID() const
{
	return m_tex->TexID();
}

int Palette::GetTexWidth() const
{
	return m_tex->Width();
}

int Palette::GetTexHeight() const
{
	return m_tex->Height();
}


}