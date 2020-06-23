#include "tessellation/Palette.h"

#include <unirender/Device.h>
#include <unirender/TextureDescription.h>

namespace tess
{

// b w
// r g

const sm::vec2 Palette::UV_RED   = sm::vec2(0.25f, 0.25f);
const sm::vec2 Palette::UV_GREEN = sm::vec2(0.75f, 0.25f);
const sm::vec2 Palette::UV_BLUE  = sm::vec2(0.25f, 0.75f);
const sm::vec2 Palette::UV_WHITE = sm::vec2(0.75f, 0.75f);

Palette::Palette(const ur::Device& dev)
{
	uint32_t buf[4];
	buf[0] = 0xff0000ff; // r
	buf[1] = 0xff00ff00; // g
	buf[2] = 0xffff0000; // b
	buf[3] = 0xffffffff; // w

    m_tex = dev.CreateTexture(2, 2, ur::TextureFormat::RGBA8, buf, 2 * 2 * 4);
}

}