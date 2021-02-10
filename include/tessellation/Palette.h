#pragma once

#include <SM_Vector.h>
#include <SM_Rect.h>
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
    auto GetRelocatedTex() const { return m_relocated_tex; }

    sm::vec2 GetWhiteUV() const;
    static sm::vec2 GetWhiteUVDefault();

    void RelocateUV(const ur::TexturePtr& tex,
        const sm::rect& region);

private:
    ur::TexturePtr m_tex = nullptr;
    ur::TexturePtr m_relocated_tex = nullptr;

    sm::vec2 m_uv_white;

}; // Palette

}