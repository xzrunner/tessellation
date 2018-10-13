#pragma once

#include <SM_Vector.h>

#include <vector>

namespace prim { class Path; }

namespace tess
{

static const uint32_t CORNER_FLAGS_TOP_LEFT  = 0x1;
static const uint32_t CORNER_FLAGS_TOP_RIGHT = 0x2;
static const uint32_t CORNER_FLAGS_BOT_LEFT  = 0x4;
static const uint32_t CORNER_FLAGS_BOT_RIGHT = 0x8;
static const uint32_t CORNER_FLAGS_TOP   = CORNER_FLAGS_TOP_LEFT | CORNER_FLAGS_TOP_RIGHT;
static const uint32_t CORNER_FLAGS_BOT   = CORNER_FLAGS_BOT_LEFT | CORNER_FLAGS_BOT_RIGHT;
static const uint32_t CORNER_FLAGS_LEFT  = CORNER_FLAGS_TOP_LEFT | CORNER_FLAGS_BOT_LEFT;
static const uint32_t CORNER_FLAGS_RIGHT = CORNER_FLAGS_TOP_RIGHT | CORNER_FLAGS_BOT_RIGHT;
static const uint32_t CORNER_FLAGS_ALL   = 0xF;

static const uint32_t ANTI_ALIASED_LINES = 0x1;
static const uint32_t ANTI_ALIASED_FILL  = 0x2;

class Painter
{
public:

	void AddLine(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, float size = 1.0f);
	void AddRect(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, uint32_t rounding = CORNER_FLAGS_ALL, float size = 1.0f);
	void AddRectFilled(const sm::vec2& p0, const sm::vec2& p1, uint32_t col, uint32_t rounding = CORNER_FLAGS_ALL);

	void AddPolyline(const sm::vec2* points, size_t count, uint32_t col, float size = 1.0f);
	void AddPolygon(const sm::vec2* points, size_t count, uint32_t col, float size = 1.0f);
	void AddPolygonFilled(const sm::vec2* points, size_t count, uint32_t col);

public:
	struct Vertex
	{
		sm::vec2 pos;
		sm::vec2 uv;
		uint32_t col = 0;
	};

	struct Cmd
	{
		size_t elem_count = 0;
	};

	struct Buffer
	{
		void Reserve(size_t idx_count, size_t vtx_count);

		void Clear();

		std::vector<Cmd>            commands;
		std::vector<Vertex>         vertices;
		std::vector<unsigned short> indices;

		unsigned short  curr_index = 0;
		Vertex*         vert_ptr = nullptr;
		unsigned short* index_ptr = nullptr;
	};

	auto& GetBuffer() const { return m_buf; }

private:
	void Stroke(const sm::vec2* points, size_t count, uint32_t col, bool closed, float size = 1.0f);
	void Fill(const sm::vec2* points, size_t count, uint32_t col);

	void Stroke(const prim::Path& path, uint32_t col, float size = 1.0f);
	void Fill(const prim::Path& path, uint32_t col);

private:
	uint32_t m_flags = ANTI_ALIASED_LINES | ANTI_ALIASED_FILL;

	Buffer m_buf;

}; // Painter

}