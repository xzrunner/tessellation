#include "tessellation/Palette.h"

#include <unirender2/Device.h>
#include <unirender2/TextureDescription.h>
#include <unirender2/Bitmap.h>

namespace tess
{

// b w
// r g

const sm::vec2 Palette::UV_RED   = sm::vec2(0.25f, 0.25f);
const sm::vec2 Palette::UV_GREEN = sm::vec2(0.75f, 0.25f);
const sm::vec2 Palette::UV_BLUE  = sm::vec2(0.25f, 0.75f);
const sm::vec2 Palette::UV_WHITE = sm::vec2(0.75f, 0.75f);

Palette::Palette(const ur2::Device& dev)
{
	uint32_t buf[4];
	buf[0] = 0xff0000ff; // r
	buf[1] = 0xff00ff00; // g
	buf[2] = 0xffff0000; // b
	buf[3] = 0xffffffff; // w

    ur2::Bitmap bmp(2, 2, 4, reinterpret_cast<unsigned char*>(buf));
    m_tex = dev.CreateTexture(bmp, ur2::TextureFormat::RGBA8);
}

}