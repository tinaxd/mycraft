#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>
#include <memory>

namespace mycraft
{

using block_id_t = std::uint8_t;

class Block
{
public:
	explicit Block(block_id_t block_id);
	Block() :
			Block(0)
	{
	}

	block_id_t block_id() const
	{
		return block_id_;
	}

	void set_block_id(block_id_t block_id)
	{
		block_id_ = block_id;
	}

private:
	block_id_t block_id_;
};

}
