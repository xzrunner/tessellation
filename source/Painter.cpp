#include "tessellation/Painter.h"
#include "tessellation/Palette.h"

#include <SM_Calc.h>
#include <primitive/Path.h>

#include <array>
#include <iterator>
#include <cmath>

namespace
{
const uint32_t COL32_A_MASK = 0xFF000000;
}

namespace tess
{

Painter::Painter(const Painter& pt)
	: m_flags(pt.m_flags)
	, m_buf(pt.m_buf)
	, m_other_texs(pt.m_other_texs)
	, m_palette(pt.m_palette)
{
}

Painter& Painter::operator = (const Painter& pt)
{
	m_flags      = pt.m_flags;
	m_buf        = pt.m_buf;
	m_other_texs = pt.m_other_texs;
	m_palette    = pt.m_palette;
	return *this;
}

void Painter::AddLine(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	sm::vec2 pts[] = { p0, p1 };
	Stroke(pts, 2, col, false, line_width);
}

void Painter::AddDashLine(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, float line_width, float step_len)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}
	if (p0 == p1) {
		return;
	}

	const float tot_len = sm::dis_pos_to_pos(p0, p1);
	const sm::vec2 dt = (p1 - p0) / tot_len;
	float len = 0;
	while (len < tot_len)
	{
		const float s = len;
		const float e = std::min(tot_len, len + step_len);
		sm::vec2 pts[] = { dt * s, dt * e };
		Stroke(pts, 2, col, false, line_width);
		len += step_len * 2;
	}
}

void Painter::AddRect(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, float line_width, float rounding, uint32_t rounding_corners_flags)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	auto path = PathRect(p0, p1, col, rounding, rounding_corners_flags);
	Stroke(path, col, line_width);
}

void Painter::AddRectFilled(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, float rounding, uint32_t rounding_corners_flags)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	auto path = PathRect(p0, p1, col, rounding, rounding_corners_flags);
	Fill(path, col);
}

void Painter::AddRectFilled(const sm::vec2& center, float radius, uint32_t col, float rounding, uint32_t rounding_corners_flags)
{
    AddRectFilled(sm::vec2(center.x - radius, center.y - radius), sm::vec2(center.x + radius, center.y + radius), col, rounding, rounding_corners_flags);
}

void Painter::AddCircle(const sm::vec2& centre, float radius, uint32_t col, float line_width, uint32_t num_segments)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	prim::Path path;
	path.Arc(centre, radius - 0.5f, 0.0f, SM_PI * 2.0f, num_segments);
	Stroke(path, col, line_width);
}

void Painter::AddCircleFilled(const sm::vec2& centre, float radius, uint32_t col, uint32_t num_segments)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	prim::Path path;
	path.Arc(centre, radius - 0.5f, 0.0f, SM_PI * 2.0f, num_segments);
	Fill(path, col);
}

void Painter::AddArc(const sm::vec2& centre, float radius, float start_angle, float end_angle, uint32_t col, float line_width, uint32_t num_segments)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	const int num = static_cast<int>(std::ceil(std::abs(start_angle - end_angle) / SM_PI * 2.0f * num_segments));

	prim::Path path;
	path.Arc(centre, radius - 0.5f, start_angle, end_angle, num);
	Stroke(path, col, line_width);
}

void Painter::AddTriangle(const sm::vec2& p0, const sm::vec2& p1, const sm::vec2& p2, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	prim::Path path;
	path.MoveTo(p0);
	path.LineTo(p1);
	path.LineTo(p2);
	path.LineTo(p0);
	Stroke(path, col, line_width);
}

void Painter::AddTriangleFilled(const sm::vec2& p0, const sm::vec2& p1, const sm::vec2& p2, uint32_t col)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	prim::Path path;
	path.MoveTo(p0);
	path.LineTo(p1);
	path.LineTo(p2);
	path.LineTo(p0);
	Fill(path, col);
}

void Painter::AddPolyline(const sm::vec2* points, size_t count, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	Stroke(points, count, col, false, line_width);
}

void Painter::AddPolylineMultiColor(const sm::vec2* points, const uint32_t* cols, size_t count, float line_width)
{
	StrokeMultiColor(points, cols, count, false, line_width);
}

void Painter::AddPolylineDash(const sm::vec2* points, size_t count, uint32_t col, float line_width, float step_len)
{
    if ((col & COL32_A_MASK) == 0 || count < 2) {
        return;
    }

    bool draw = true;
    std::vector<sm::vec2> buf;
    buf.push_back(points[0]);
    float need = step_len;
    int ptr = 0;
    float seg_len = sm::dis_pos_to_pos(points[0], points[1]);
    float seg_len_left = seg_len;
    while (ptr < static_cast<int>(count) - 1)
    {
        if (need <= seg_len_left)
        {
            seg_len_left -= need;
            need = step_len;
            auto pos = points[ptr + 1] + (points[ptr] - points[ptr + 1]) * (seg_len_left / seg_len);
            buf.push_back(pos);
            if (draw) {
                AddPolyline(buf.data(), buf.size(), col, line_width);
            }
            draw = !draw;
            buf.clear();
            buf.push_back(pos);
        }
        else
        {
            buf.push_back(points[ptr + 1]);
            need -= seg_len_left;
            ++ptr;
            seg_len = sm::dis_pos_to_pos(points[ptr], points[ptr + 1]);
            seg_len_left = seg_len;
        }
    }
    if (draw) {
        AddPolyline(buf.data(), buf.size(), col, line_width);
    }
}

void Painter::AddPolygon(const sm::vec2* points, size_t count, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	Stroke(points, count, col, true, line_width);
}

void Painter::AddPolygonFilled(const sm::vec2* points, size_t count, uint32_t col)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	Fill(points, count, col);
}

void Painter::AddPath(const prim::Path& path, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	Stroke(path, col, line_width);
}

void Painter::AddPoint3D(const sm::vec3& p, Trans2dFunc trans, uint32_t col, float size)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	AddCircleFilled(trans(p), size, col);
}

void Painter::AddLine3D(const sm::vec3& p0, const sm::vec3& p1, Trans2dFunc trans, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	sm::vec2 vs[] = { trans(p0), trans(p1) };
	Stroke(vs, 2, col, false, line_width);
}

void Painter::AddCube(const sm::cube& cube, Trans2dFunc trans, uint32_t col, float line_width)
{
	auto& min = cube.min;
	auto& max = cube.max;
	std::array<sm::vec3, 8> v3 = {
		sm::vec3(min[0], min[1], min[2]),
		sm::vec3(max[0], min[1], min[2]),
		sm::vec3(max[0], max[1], min[2]),
		sm::vec3(min[0], max[1], min[2]),
		sm::vec3(min[0], min[1], max[2]),
		sm::vec3(max[0], min[1], max[2]),
		sm::vec3(max[0], max[1], max[2]),
		sm::vec3(min[0], max[1], max[2])
	};

	std::array<sm::vec2, 8> v2;
	for (int i = 0; i < 8; ++i) {
		v2[i] = trans(v3[i]);
	}

	// bottom
	AddLine(v2[0], v2[1], col, line_width);
	AddLine(v2[1], v2[2], col, line_width);
	AddLine(v2[2], v2[3], col, line_width);
	AddLine(v2[3], v2[0], col, line_width);
	// top
	AddLine(v2[4], v2[5], col, line_width);
	AddLine(v2[5], v2[6], col, line_width);
	AddLine(v2[6], v2[7], col, line_width);
	AddLine(v2[7], v2[4], col, line_width);
	// middle
	AddLine(v2[0], v2[4], col, line_width);
	AddLine(v2[1], v2[5], col, line_width);
	AddLine(v2[2], v2[6], col, line_width);
	AddLine(v2[3], v2[7], col, line_width);
}

void Painter::AddArc3D(const sm::mat4& mat, float radius, float start_angle, float end_angle,
	                   Trans2dFunc trans, uint32_t col, float line_width, uint32_t num_segments)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	std::vector<sm::vec2> vertices;
	vertices.resize(num_segments);
	for (size_t i = 0; i < num_segments; ++i)
	{
		float angle = start_angle + (end_angle - start_angle) * (static_cast<float>(i) / (num_segments - 1));
		auto pos3 = mat * (sm::mat4::RotatedZ(angle * SM_RAD_TO_DEG) * sm::vec3(radius, 0, 0));
		vertices[i] = trans(pos3);
	}

	Stroke(vertices.data(), vertices.size(), col, false, line_width);
}

void Painter::AddPolyline3D(const sm::vec3* points, size_t count, Trans2dFunc trans, uint32_t col, float line_width, bool closed)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	std::vector<sm::vec2> vs2;
	vs2.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		vs2.push_back(trans(points[i]));
	}
    if (closed && !vs2.empty()) {
        vs2.push_back(vs2.front());
        count += 1;
    }

	Stroke(vs2.data(), count, col, false, line_width);
}

void Painter::AddPolygon3D(const sm::vec3* points, size_t count, Trans2dFunc trans, uint32_t col, float line_width)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	std::vector<sm::vec2> vs2;
	vs2.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		vs2.push_back(trans(points[i]));
	}

	Stroke(vs2.data(), count, col, true, line_width);
}

void Painter::AddPolygonFilled3D(const sm::vec3* points, size_t count, Trans2dFunc trans, uint32_t col)
{
	if ((col & COL32_A_MASK) == 0) {
		return;
	}

	std::vector<sm::vec2> vs2;
	vs2.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		vs2.push_back(trans(points[i]));
	}

	Fill(vs2.data(), count, col);
}

void Painter::AddTexQuad(int tex, const std::array<sm::vec2, 4>& positions, const std::array<sm::vec2, 4>& texcoords, uint32_t color)
{
	bool merged = false;
	if (!m_other_texs.empty())
	{
		auto& last = m_other_texs.back();
		if (last.texid == tex && last.end + 1 == m_buf.curr_index) {
			last.end += 4;
			merged = true;
		}
	}
	if (!merged) {
		m_other_texs.push_back({ tex, m_buf.curr_index, m_buf.curr_index + 3 });
	}

	m_buf.Reserve(6, 4);

	m_buf.index_ptr[0] = m_buf.curr_index;
	m_buf.index_ptr[1] = m_buf.curr_index + 1;
	m_buf.index_ptr[2] = m_buf.curr_index + 2;
	m_buf.index_ptr[3] = m_buf.curr_index;
	m_buf.index_ptr[4] = m_buf.curr_index + 2;
	m_buf.index_ptr[5] = m_buf.curr_index + 3;
	m_buf.index_ptr += 6;

	for (int i = 0; i < 4; ++i)
	{
		auto& v = m_buf.vert_ptr[i];
		v.pos = positions[i];
		v.uv  = texcoords[i];
		v.col = color;
	}
	m_buf.vert_ptr += 4;

	m_buf.curr_index += 4;
}

void Painter::AddPainter(const Painter& pt)
{
	auto& buf = pt.GetBuffer();
	if (buf.indices.empty()) {
		return;
	}

	const size_t idx_count = buf.indices.size();
	const size_t vtx_count = buf.vertices.size();
	m_buf.Reserve(idx_count, vtx_count);
	auto off_vert = m_buf.curr_index;
	for (size_t i = 0; i < idx_count; ++i) {
		*m_buf.index_ptr++ = buf.indices[i] + off_vert;
	}
	for (size_t i = 0; i < vtx_count; ++i) {
		*m_buf.vert_ptr++ = buf.vertices[i];
	}
	m_buf.curr_index += static_cast<unsigned short>(vtx_count);

	auto off_tex = m_other_texs.size();
	std::copy(pt.m_other_texs.begin(), pt.m_other_texs.end(), std::back_inserter(m_other_texs));
	for (int i = off_tex, n = m_other_texs.size(); i < n; ++i) {
		auto& tex = m_other_texs[i];
		tex.begin += off_vert;
		tex.end   += off_vert;
	}
}

void Painter::FillPainter(const Painter& pt, size_t vert_off, size_t index_off, size_t tex_off)
{
	auto& buf = pt.GetBuffer();
	if (buf.indices.empty() || buf.vertices.empty()) {
		return;
	}

	auto& tex_region = pt.GetOtherTexRegion();

	const size_t idx_count = buf.indices.size();
	const size_t vtx_count = buf.vertices.size();
	const size_t tex_count = tex_region.size();
	assert(vert_off + vtx_count - 1 < m_buf.vertices.size()
	    && index_off + idx_count - 1 < m_buf.indices.size()
	    && (tex_count == 0 || tex_off + tex_count - 1 < m_other_texs.size()));
	auto start_index = static_cast<unsigned short>(vert_off);
	for (size_t i = 0; i < idx_count; ++i) {
		m_buf.indices[index_off + i] = buf.indices[i] + start_index;
	}
	for (size_t i = 0; i < vtx_count; ++i) {
		m_buf.vertices[vert_off + i] = buf.vertices[i];
	}
	for (size_t i = 0; i < tex_count; ++i) {
		auto& dst = m_other_texs[tex_off + i];
		dst = tex_region[i];
		dst.begin += start_index;
		dst.end += start_index;
	}
}

bool Painter::IsEmpty() const
{
	return m_buf.indices.empty();
}

void Painter::Clear()
{
	m_buf.Clear();
	m_other_texs.clear();
}

void Painter::SetAntiAliased(bool enable)
{
    if (enable) {
        m_flags |= ANTI_ALIASED_LINES;
        m_flags |= ANTI_ALIASED_FILL;
    } else {
        m_flags &= ~ANTI_ALIASED_LINES;
        m_flags &= ~ANTI_ALIASED_FILL;
    }
}

prim::Path Painter::PathRect(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, float rounding, uint32_t rounding_corners_flags)
{
	prim::Path path;
	if (rounding > 0.0f && rounding_corners_flags != CORNER_FLAGS_NONE)
	{
		const size_t num_seg = 6;
		const float rounding_tl = (rounding_corners_flags & CORNER_FLAGS_TOP_LEFT)  ? rounding : 0.0f;
		const float rounding_tr = (rounding_corners_flags & CORNER_FLAGS_TOP_RIGHT) ? rounding : 0.0f;
		const float rounding_br = (rounding_corners_flags & CORNER_FLAGS_BOT_RIGHT) ? rounding : 0.0f;
		const float rounding_bl = (rounding_corners_flags & CORNER_FLAGS_BOT_LEFT)  ? rounding : 0.0f;
		path.Arc(sm::vec2(p1.x - rounding_tr, p1.y - rounding_tr), rounding_tr, 0, SM_PI * 0.5f, num_seg);
		path.Arc(sm::vec2(p0.x + rounding_tl, p1.y - rounding_tl), rounding_tl, SM_PI * 0.5f, SM_PI, num_seg);
		path.Arc(sm::vec2(p0.x + rounding_bl, p0.y + rounding_bl), rounding_bl, SM_PI, SM_PI * 1.5f, num_seg);
		path.Arc(sm::vec2(p1.x - rounding_br, p0.y + rounding_br), rounding_br, SM_PI * 1.5f, SM_PI * 2, num_seg);
	}
	else
	{
		path.MoveTo(p0);
		path.LineTo({ p1.x, p0.y });
		path.LineTo(p1);
		path.LineTo({ p0.x, p1.y });
		path.LineTo(p0);
	}
	return path;
}

void Painter::Stroke(const sm::vec2* points, size_t ori_count, uint32_t col, bool closed, float line_width)
{
	if ((col & COL32_A_MASK) == 0 || ori_count < 2) {
		return;
	}

	std::vector<uint32_t> colors(ori_count, col);
	StrokeMultiColor(points, colors.data(), ori_count, closed, line_width);
}

// code from imgui: https://github.com/ocornut/imgui
void Painter::StrokeMultiColor(const sm::vec2* points, const uint32_t* cols, size_t ori_count, bool closed, float line_width)
{
	size_t new_count = closed ? ori_count : ori_count - 1;

	auto& uv = m_palette ? m_palette->GetWhiteUV() : Palette::GetWhiteUVDefault();
	if (m_flags & ANTI_ALIASED_LINES)
	{
		const bool thick_line = line_width > 1.0f;

        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;

        const int idx_count = thick_line ? new_count * 18 : new_count * 12;
        const int vtx_count = thick_line ? ori_count * 4 : ori_count * 3;
		m_buf.Reserve(idx_count, vtx_count);

        // Temporary buffer
		sm::vec2* temp_normals = (sm::vec2*)alloca(ori_count * (thick_line ? 5 : 3) * sizeof(sm::vec2));
		sm::vec2* temp_points = temp_normals + ori_count;

        for (size_t i1 = 0; i1 < new_count; i1++)
        {
            const int i2 = (i1+1) == ori_count ? 0 : i1+1;
			auto& p0 = points[i1];
			auto& p1 = points[i2];
			auto diff = p1 - p0;
			auto inv_len = p0 == p1 ? 1 : 1.0f / sm::dis_pos_to_pos(p0, p1);
			diff *= inv_len;
            temp_normals[i1].x = diff.y;
            temp_normals[i1].y = -diff.x;
        }
        if (!closed)
            temp_normals[ori_count - 1] = temp_normals[ori_count - 2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(ori_count-1)*2+0] = points[ori_count-1] + temp_normals[ori_count-1] * AA_SIZE;
                temp_points[(ori_count-1)*2+1] = points[ori_count-1] - temp_normals[ori_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = m_buf.curr_index;
            for (size_t i1 = 0; i1 < new_count; i1++)
            {
                const int i2 = (i1+1) == ori_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == ori_count ? m_buf.curr_index : idx1+3;

                // Average normals
                sm::vec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                dm *= AA_SIZE;
                temp_points[i2*2+0] = points[i2] + dm;
                temp_points[i2*2+1] = points[i2] - dm;

                // Add indexes
                m_buf.index_ptr[0] = (idx2+0); m_buf.index_ptr[1] = (idx1+0); m_buf.index_ptr[2] = (idx1+2);
                m_buf.index_ptr[3] = (idx1+2); m_buf.index_ptr[4] = (idx2+2); m_buf.index_ptr[5] = (idx2+0);
                m_buf.index_ptr[6] = (idx2+1); m_buf.index_ptr[7] = (idx1+1); m_buf.index_ptr[8] = (idx1+0);
                m_buf.index_ptr[9] = (idx1+0); m_buf.index_ptr[10]= (idx2+0); m_buf.index_ptr[11]= (idx2+1);
                m_buf.index_ptr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (size_t i = 0; i < ori_count; i++)
            {
				auto& col = cols[i];
				const uint32_t col_trans = col & ~COL32_A_MASK;
                m_buf.vert_ptr[0].pos = points[i];          m_buf.vert_ptr[0].uv = uv; m_buf.vert_ptr[0].col = col;
                m_buf.vert_ptr[1].pos = temp_points[i*2+0]; m_buf.vert_ptr[1].uv = uv; m_buf.vert_ptr[1].col = col_trans;
                m_buf.vert_ptr[2].pos = temp_points[i*2+1]; m_buf.vert_ptr[2].uv = uv; m_buf.vert_ptr[2].col = col_trans;
                m_buf.vert_ptr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (line_width - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(ori_count-1)*4+0] = points[ori_count-1] + temp_normals[ori_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(ori_count-1)*4+1] = points[ori_count-1] + temp_normals[ori_count-1] * (half_inner_thickness);
                temp_points[(ori_count-1)*4+2] = points[ori_count-1] - temp_normals[ori_count-1] * (half_inner_thickness);
                temp_points[(ori_count-1)*4+3] = points[ori_count-1] - temp_normals[ori_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = m_buf.curr_index;
            for (size_t i1 = 0; i1 < new_count; i1++)
            {
                const int i2 = (i1+1) == ori_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == ori_count ? m_buf.curr_index : idx1+4;

                // Average normals
                sm::vec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                sm::vec2 dm_out = dm * (half_inner_thickness + AA_SIZE);
                sm::vec2 dm_in = dm * half_inner_thickness;
                temp_points[i2*4+0] = points[i2] + dm_out;
                temp_points[i2*4+1] = points[i2] + dm_in;
                temp_points[i2*4+2] = points[i2] - dm_in;
                temp_points[i2*4+3] = points[i2] - dm_out;

                // Add indexes
                m_buf.index_ptr[0]  = (idx2+1); m_buf.index_ptr[1]  = (idx1+1); m_buf.index_ptr[2]  = (idx1+2);
                m_buf.index_ptr[3]  = (idx1+2); m_buf.index_ptr[4]  = (idx2+2); m_buf.index_ptr[5]  = (idx2+1);
                m_buf.index_ptr[6]  = (idx2+1); m_buf.index_ptr[7]  = (idx1+1); m_buf.index_ptr[8]  = (idx1+0);
                m_buf.index_ptr[9]  = (idx1+0); m_buf.index_ptr[10] = (idx2+0); m_buf.index_ptr[11] = (idx2+1);
                m_buf.index_ptr[12] = (idx2+2); m_buf.index_ptr[13] = (idx1+2); m_buf.index_ptr[14] = (idx1+3);
                m_buf.index_ptr[15] = (idx1+3); m_buf.index_ptr[16] = (idx2+3); m_buf.index_ptr[17] = (idx2+2);
                m_buf.index_ptr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (size_t i = 0; i < ori_count; i++)
            {
				auto& col = cols[i];
				const uint32_t col_trans = col & ~COL32_A_MASK;
                m_buf.vert_ptr[0].pos = temp_points[i*4+0]; m_buf.vert_ptr[0].uv = uv; m_buf.vert_ptr[0].col = col_trans;
                m_buf.vert_ptr[1].pos = temp_points[i*4+1]; m_buf.vert_ptr[1].uv = uv; m_buf.vert_ptr[1].col = col;
                m_buf.vert_ptr[2].pos = temp_points[i*4+2]; m_buf.vert_ptr[2].uv = uv; m_buf.vert_ptr[2].col = col;
                m_buf.vert_ptr[3].pos = temp_points[i*4+3]; m_buf.vert_ptr[3].uv = uv; m_buf.vert_ptr[3].col = col_trans;
                m_buf.vert_ptr += 4;
            }
        }
        m_buf.curr_index += vtx_count;
	}
	else
	{
		const size_t idx_count = new_count * 6;
		const size_t vtx_count = new_count * 4;
		m_buf.Reserve(idx_count, vtx_count);

		for (size_t i = 0; i < new_count; ++i)
		{
			auto& col = cols[i];

			const int j = (i + 1) == ori_count ? 0 : i + 1;
			auto& p0 = points[i];
			auto& p1 = points[j];
			auto diff = p1 - p0;
			auto inv_len = p0 == p1 ? 1 : 1.0f / sm::dis_pos_to_pos(p0, p1);
			diff *= inv_len;

			const float dx = diff.x * (line_width * 0.5f);
			const float dy = diff.y * (line_width * 0.5f);
			m_buf.vert_ptr[0].pos.x = p0.x + dy; m_buf.vert_ptr[0].pos.y = p0.y - dx; m_buf.vert_ptr[0].uv = uv; m_buf.vert_ptr[0].col = col;
			m_buf.vert_ptr[1].pos.x = p1.x + dy; m_buf.vert_ptr[1].pos.y = p1.y - dx; m_buf.vert_ptr[1].uv = uv; m_buf.vert_ptr[1].col = col;
			m_buf.vert_ptr[2].pos.x = p1.x - dy; m_buf.vert_ptr[2].pos.y = p1.y + dx; m_buf.vert_ptr[2].uv = uv; m_buf.vert_ptr[2].col = col;
			m_buf.vert_ptr[3].pos.x = p0.x - dy; m_buf.vert_ptr[3].pos.y = p0.y + dx; m_buf.vert_ptr[3].uv = uv; m_buf.vert_ptr[3].col = col;
			m_buf.vert_ptr += 4;

			m_buf.index_ptr[0] = m_buf.curr_index;
			m_buf.index_ptr[1] = m_buf.curr_index + 1;
			m_buf.index_ptr[2] = m_buf.curr_index + 2;
			m_buf.index_ptr[3] = m_buf.curr_index;
			m_buf.index_ptr[4] = m_buf.curr_index + 2;
			m_buf.index_ptr[5] = m_buf.curr_index + 3;
			m_buf.index_ptr  += 6;
			m_buf.curr_index += 4;
		}
	}
}

// code from imgui: https://github.com/ocornut/imgui
void Painter::Fill(const sm::vec2* points, size_t count, uint32_t col)
{
	if ((col & COL32_A_MASK) == 0 || count < 3) {
		return;
	}

	auto& uv = m_palette ? m_palette->GetWhiteUV() : Palette::GetWhiteUVDefault();
	if (m_flags & ANTI_ALIASED_FILL)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const uint32_t col_trans = col & ~COL32_A_MASK;
        const int idx_count = (count - 2) * 3 + count * 6;
        const int vtx_count = (count * 2);
		m_buf.Reserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = m_buf.curr_index;
        unsigned int vtx_outer_idx = m_buf.curr_index+1;
        for (int i = 2; i < static_cast<int>(count); i++)
        {
            m_buf.index_ptr[0] = vtx_inner_idx;
			m_buf.index_ptr[1] = vtx_inner_idx + ((i - 1) << 1);
			m_buf.index_ptr[2] = vtx_inner_idx + (i << 1);
            m_buf.index_ptr += 3;
        }

        // Compute normals
        sm::vec2* temp_normals = (sm::vec2*)alloca(count * sizeof(sm::vec2));
        for (int i0 = count-1, i1 = 0; i1 < static_cast<int>(count); i0 = i1++)
        {
            const sm::vec2& p0 = points[i0];
            const sm::vec2& p1 = points[i1];
            sm::vec2 diff = p1 - p0;
			auto inv_len = p0 == p1 ? 1 : 1.0f / sm::dis_pos_to_pos(p0, p1);
			diff *= inv_len;
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = count-1, i1 = 0; i1 < static_cast<int>(count); i0 = i1++)
        {
            // Average normals
            const sm::vec2& n0 = temp_normals[i0];
            const sm::vec2& n1 = temp_normals[i1];
            sm::vec2 dm = (n0 + n1) * 0.5f;
			float dmr2 = dm.LengthSquared();
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            m_buf.vert_ptr[0].pos = (points[i1] - dm); m_buf.vert_ptr[0].uv = uv; m_buf.vert_ptr[0].col = col;        // Inner
            m_buf.vert_ptr[1].pos = (points[i1] + dm); m_buf.vert_ptr[1].uv = uv; m_buf.vert_ptr[1].col = col_trans;  // Outer
            m_buf.vert_ptr += 2;

            // Add indexes for fringes
            m_buf.index_ptr[0] = vtx_inner_idx + (i1 << 1);
			m_buf.index_ptr[1] = vtx_inner_idx + (i0 << 1);
			m_buf.index_ptr[2] = vtx_outer_idx + (i0 << 1);
            m_buf.index_ptr[3] = vtx_outer_idx + (i0 << 1);
			m_buf.index_ptr[4] = vtx_outer_idx + (i1 << 1);
			m_buf.index_ptr[5] = vtx_inner_idx + (i1 << 1);
            m_buf.index_ptr += 6;
        }
        m_buf.curr_index += vtx_count;
    }
	else
	{
		const size_t idx_count = (count - 2) * 3;
		const size_t vtx_count = count;
		m_buf.Reserve(idx_count, vtx_count);
		for (size_t i = 0; i < vtx_count; i++)
		{
			m_buf.vert_ptr[0].pos = points[i];
			m_buf.vert_ptr[0].uv = uv;
			m_buf.vert_ptr[0].col = col;
			m_buf.vert_ptr++;
		}
		for (unsigned short i = 2; i < count; i++)
		{
			m_buf.index_ptr[0] = m_buf.curr_index;
			m_buf.index_ptr[1] = m_buf.curr_index + i - 1;
			m_buf.index_ptr[2] = m_buf.curr_index + i;
			m_buf.index_ptr += 3;
		}
		m_buf.curr_index += static_cast<unsigned short>(vtx_count);
	}
}

void Painter::Stroke(const prim::Path& path, uint32_t col, float line_width)
{
	for (auto& path : path.GetPrevPaths()) {
		Stroke(path.vertices.data(), path.vertices.size(), col, false, line_width);
	}
	auto& p = path.GetCurrPath();
	Stroke(p.data(), p.size(), col, false, line_width);
}

void Painter::Fill(const prim::Path& path, uint32_t col)
{
	for (auto& path : path.GetPrevPaths()) {
		Fill(path.vertices.data(), path.vertices.size(), col);
	}
	auto& p = path.GetCurrPath();
	Fill(p.data(), p.size() - 1, col);
}

//////////////////////////////////////////////////////////////////////////
// struct Painter::Buffer
//////////////////////////////////////////////////////////////////////////

Painter::Buffer::Buffer(const Buffer& buf)
	: vertices(buf.vertices)
	, indices(buf.indices)
	, curr_index(buf.curr_index)
{
	vert_ptr  = vertices.data() + vertices.size();
	index_ptr = indices.data() + indices.size();
}

Painter::Buffer& Painter::Buffer::operator = (const Buffer& buf)
{
	vertices   = buf.vertices;
	indices    = buf.indices;
	curr_index = buf.curr_index;
	vert_ptr   = vertices.data() + vertices.size();
	index_ptr  = indices.data() + indices.size();
	return *this;
}

void Painter::Buffer::Reserve(size_t idx_count, size_t vtx_count)
{
	//assert(!commands.empty());
	//commands.back().elem_count += idx_count;

	size_t sz = vertices.size();
	vertices.resize(sz + vtx_count);
	vert_ptr = vertices.data() + sz;

	sz = indices.size();
	indices.resize(sz + idx_count);
	index_ptr = indices.data() + sz;
}

void Painter::Buffer::Clear()
{
//	commands.resize(0);
	vertices.resize(0);
	indices.resize(0);

	curr_index = 0;
	vert_ptr   = nullptr;
	index_ptr  = nullptr;
}

}