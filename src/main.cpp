/*
 * main.cpp
 *
 *  Created on: 2021/02/22
 *      Author: tinaxd
 */

#include "graphics.h"
#include "worldgen.h"
#include "world.h"
#include "TextureMap.h"

using namespace mycraft;

int main(int argc, char **argv)
{
	//WorldGenerator gen;
	//const auto& chunk = gen.generate_chunk(0, 0, 0);

	std::array<Block, Chunk::chunk_length*Chunk::chunk_length*Chunk::chunk_height> blks;
	for (int i=0; i<Chunk::chunk_length; i++)
		for (int j=0; j<Chunk::chunk_length; j++)
			for (int k=0; k<Chunk::chunk_height; k++) {
				int id = k == Chunk::chunk_height-1 ? 2 : 1;
				blks[Chunk::convert_index(i, j, k)].set_block_id(id);
			}
	auto chunk = std::make_shared<Chunk>(blks);

	std::array<Block, Chunk::chunk_length*Chunk::chunk_length*Chunk::chunk_height> blks2;
		for (int i=0; i<Chunk::chunk_length; i++)
			for (int j=0; j<Chunk::chunk_length; j++)
				for (int k=0; k<Chunk::chunk_height; k++) {
					int id = k == Chunk::chunk_height-1 ? 2 : 1;
					blks2[Chunk::convert_index(i, j, k)].set_block_id(id);
				}
	auto chunk2 = std::make_shared<Chunk>(blks2);

	auto sts = std::make_shared<TextureStorage>(standard_texture_storage());

	auto world = std::make_shared<World>();
	world->set_chunk({0, 0, 0}, chunk);
	world->set_chunk({1, 0, 0}, chunk2);

	Renderer renderer(800, 600, "MyCraft");

	renderer.set_texture_storage(std::move(sts));
	renderer.set_world(std::move(world));
	renderer.render_loop();

	return 0;
}

