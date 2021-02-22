#pragma once

#include <random>
#include <cstdint>
#include <memory>
#include "block.h"
#include "world.h"

namespace mycraft
{
//class ChunkData
//{
//public:
//	constexpr static int chunk_size = 16;
//	constexpr static int max_z = 256;
//
//	ChunkData();
//
//	Block& query_block(int rx, int ry, int rz)
//	{
//		return _blocks[rx * chunk_size * max_z + ry * max_z + rz];
//	}
//
//	const Block& query_block(int rx, int ry, int rz) const
//	{
//		return _blocks[rx * chunk_size * max_z + ry * max_z + rz];
//	}
//
//	Block& operator()(int rx, int ry, int rz)
//	{
//		return query_block(rx, ry, rz);
//	}
//
//	const Block& operator()(int rx, int ry, int rz) const
//	{
//		return query_block(rx, ry, rz);
//	}
//
//private:
//	std::unique_ptr<Block[]> _blocks;
//};

class WorldGenerator
{
public:
	WorldGenerator(std::uint32_t seed = 1);

	[[nodiscard]] Chunk generate_chunk(CoordElem base_x,
			CoordElem base_y, CoordElem base_z);
};
}
