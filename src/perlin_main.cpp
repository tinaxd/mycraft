#include "perlin.h"
#include <iostream>
#include "worldgen.h"

int main2()
{
	auto result = mycraft::perlin::perlin2d_image(16, 16, 4, 2);
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			std::cout << result[i * 16 + j];
			if (j != 15)
				std::cout << ",";
			else
				std::cout << std::endl;
		}
	}
	return 0;
}

using namespace mycraft;

int main3()
{
	WorldGenerator gen;
	const auto &chunk = gen.generate_chunk(0, 0, 0);
	const auto &mz = Chunk::chunk_height;
	const auto &cs = Chunk::chunk_length;
	for (int x = 0; x < cs; x++)
	{
		for (int z = mz - 1; z >= 0; z--)
		{
			std::cout << chunk.data()[Chunk::convert_index(x, 0, z)].block_id();
			if (z != 0)
				std::cout << ",";
			else
				std::cout << std::endl;
		}
	}
	return 0;
}
