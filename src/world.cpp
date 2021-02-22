/*
 * world.cpp
 *
 *  Created on: 2021/02/22
 *      Author: tinaxd
 */


#include "world.h"

using namespace mycraft;


Chunk::Chunk()
	: data_() {}

Chunk::Chunk(const ChunkData& data)
	: data_(data) {}

Chunk::ChunkData& Chunk::modifyData()
{
	changed_ = true;
	return data_;
}

const Chunk::ChunkData& Chunk::data() const
{
	return data_;
}
