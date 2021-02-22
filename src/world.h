#pragma once

#include <vector>
#include "block.h"
#include <map>
#include <memory>
#include <array>
#include <optional>

namespace mycraft
{

class World;
class Chunk;

using CoordElem = int32_t;

template<typename Elem>
class Coord2D
{
public:
	Coord2D(Elem x, Elem y) :
			_x(x), _y(y)
	{
	}

	const Elem& x() const
	{
		return _x;
	}
	const Elem& y() const
	{
		return _y;
	}

private:
	Elem _x, _y;
};

template<typename T, typename U>
bool operator==(const Coord2D<T> &c1, const Coord2D<U> &c2)
{
	return c1.x() == c2.x() && c1.y() == c2.y();
}

template<typename T, typename U>
bool operator!=(const Coord2D<T> &c1, const Coord2D<U> &c2)
{
	return !(c1 == c2);
}

template<typename Elem>
class Coord3D
{
public:
	Coord3D(Elem x, Elem y, Elem z) :
			_x(x), _y(y), _z(z)
	{
	}

	const Elem& x() const
	{
		return _x;
	}
	const Elem& y() const
	{
		return _y;
	}
	const Elem& z() const
	{
		return _z;
	}

private:
	Elem _x, _y, _z;
};

template<typename T, typename U>
bool operator==(const Coord3D<T> &c1, const Coord3D<U> &c2)
{
	return c1.x() == c2.x() && c1.y() == c2.y() && c1.z() == c2.z();
}

template<typename T, typename U>
bool operator!=(const Coord3D<T> &c1, const Coord3D<U> &c2)
{
	return !(c1 == c2);
}

struct Coord2DSort
{
	template<typename T, typename U>
	bool operator()(const Coord2D<T> &c1, const Coord2D<U> &c2) const
	{
		if (c1.x() != c2.x())
			return c1.x() < c2.x();
		else
			return c1.y() < c2.y();
	}
};

struct Coord3DSort
{
	template<typename T, typename U>
	bool operator()(const Coord3D<T> &c1, const Coord3D<U> &c2) const
	{
		if (c1.x() != c2.x())
			return c1.x() < c2.x();
		else if (c1.y() != c2.y())
			return c1.y() < c2.y();
		else
			return c1.z() < c2.z();
	}
};

using ChunkCoord = Coord3D<CoordElem>;

class World
{
public:
	World() {}

	std::optional<std::shared_ptr<Chunk>>
	chunk(const ChunkCoord &c) const
	{
		auto it = chunks_.find(c);
		if (it == chunks_.end()) return {};
		else return std::optional(it->second);
	}

	void set_chunk(const ChunkCoord &c, std::shared_ptr<Chunk> chunk)
	{
		chunks_.insert_or_assign(c, std::move(chunk));
	}

	void free_chunk(const ChunkCoord &c)
	{
		chunks_.erase(c);
	}

private:
	std::map<ChunkCoord, std::shared_ptr<Chunk>, Coord3DSort> chunks_;
};

class Chunk
{
public:
	using ChunkCoord = int_fast16_t;

	constexpr static ChunkCoord chunk_length = 16;
	constexpr static ChunkCoord chunk_height = 16;

	using ChunkData = std::array<Block, chunk_length*chunk_length*chunk_height>;

	Chunk();
	Chunk(const ChunkData& data);

	const ChunkData& data() const;
	ChunkData& modifyData();

	bool changed() const { return changed_; }
	void set_changed(bool changed) const { changed_ = changed; }

	inline static constexpr size_t convert_index(CoordElem x, CoordElem y, CoordElem z)
	{
		return x * chunk_length * chunk_height
				+ y * chunk_height
				+ z;
	}

private:
	ChunkData data_;

	mutable bool changed_ = false;
};

}
