#include "worldgen.h"
#include "perlin.h"
#include <cmath>
#include <array>
#include "block.h"

using namespace mycraft;

WorldGenerator::WorldGenerator(std::uint32_t seed)
{
}

template<typename Array, typename T>
void array_add(Array &a, size_t len, const T &t)
{
	for (size_t i = 0; i < len; i++)
	{
		a[i] += t;
	}
}

template<typename Array, typename T>
void array_mul(Array &a, size_t len, const T &t)
{
	for (size_t i = 0; i < len; i++)
	{
		a[i] *= t;
	}
}

template<typename Src, typename Target>
Target step(const Src &x, const Src &threashold, const Target &t,
		const Target &f)
{
	return x >= threashold ? t : f;
}

template<typename T>
int step(const T &x)
{
	return step(x, 0.5, 1, 0);
}

Chunk WorldGenerator::generate_chunk(CoordElem base_x,
		CoordElem base_y, CoordElem base_z)
{
	constexpr auto cs = Chunk::chunk_length;
	constexpr auto mz = Chunk::chunk_height;
	constexpr auto csmz = cs * mz;

	std::unique_ptr<std::array<double, cs*cs*mz>> pb(new std::array<double, cs*cs*mz>);


	for (int z = 0; z < mz; z++)
	{
		for (int i = 0; i < cs; i++)
		{
			for (int j = 0; j < cs; j++)
			{
				(*pb)[i * csmz + j * mz + z] = (double) z / mz;
			}
		}
	}

	// TODO: use base_[xyz] to generate different chunks
	auto noise = perlin::perlin3d_image(cs, cs, mz, cs / 2, 4);
	// noise is [0, 1]
	array_add(noise, cs * cs * mz, -0.3); // [-0.5, 0.5] (with bias)
	array_mul(noise, cs * cs * mz, mz);
	// noise is now [-mz/2, mz/2]

	std::array<Block, cs*cs*mz> blocks;
	for (int j = 0; j < cs; j++)
	{
		for (int i = 0; i < cs; i++)
		{
			for (int k = 0; k < mz; k++)
			{
				auto kk = (int) std::round(k + noise[i * csmz + j * mz + k]);
				if (kk >= mz)
					kk = mz - 1;
				else if (kk < 0)
					kk = 0;
				blocks[Chunk::convert_index(i, j, k)] = Block(step((*pb)[i * csmz + j * mz + kk])); // Block id 0 or 1
			}
		}
	}

	return Chunk(std::move(blocks));
}
