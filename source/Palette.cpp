#include "tessellation/Palette.h"

#include <unirender/Device.h>
#include <unirender/TextureDescription.h>

namespace
{

// b w
// r g
const sm::vec2 UV_RED   = sm::vec2(0.25f, 0.25f);
const sm::vec2 UV_GREEN = sm::vec2(0.75f, 0.25f);
const sm::vec2 UV_BLUE  = sm::vec2(0.25f, 0.75f);
const sm::vec2 UV_WHITE = sm::vec2(0.75f, 0.75f);

}

namespace tess
{

Palette::Palette(const ur::Device& dev)
	: m_uv_white(UV_WHITE)
{
	uint32_t buf[4];
	buf[0] = 0xff0000ff; // r
	buf[1] = 0xff00ff00; // g
	buf[2] = 0xffff0000; // b
	buf[3] = 0xffffffff; // w

    m_tex = dev.CreateTexture(2, 2, ur::TextureFormat::RGBA8, buf, 2 * 2 * 4);
}

sm::vec2 Palette::GetWhiteUV() const
{
	return m_uv_white;
}

sm::vec2 Palette::GetWhiteUVDefault()
{
	return UV_WHITE;
}

void Palette::RelocateUV(const ur::TexturePtr& tex, const sm::rect& region)
{
	m_relocated_tex = tex;
	m_uv_white.x = region.xmin + region.Width()  * UV_WHITE.x;
	m_uv_white.y = region.ymin + region.Height() * UV_WHITE.y;
}

}