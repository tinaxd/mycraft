/*
 * TextureMap.cpp
 *
 *  Created on: 2021/02/23
 *      Author: tinaxd
 */

#include "TextureMap.h"

#include <cassert>

using namespace mycraft;

TextureStorage::TextureMap::TextureMap(const std::vector<TextureMapCoord>& coords)
	: xneg_(coords[0])
	, yneg_(coords[1])
	, xpos_(coords[2])
	, ypos_(coords[3])
	, zneg_(coords[4])
	, zpos_(coords[5])
{
	assert(coords.size() == 6);
}

texture_id_t TextureStorage::append(TextureMap map)
{
	const auto ret = textures_.size();
	textures_.push_back(std::move(map));
	return ret;
}

TextureStorage mycraft::standard_texture_storage()
{
	using TM = TextureStorage::TextureMap;
	TextureStorage ts;
	ts.append(TM({ // tex_id=0, dirt
		{{2, 0}, {3, 1}},
		{{2, 0}, {3, 1}},
		{{2, 0}, {3, 1}},
		{{2, 0}, {3, 1}},
		{{2, 0}, {3, 1}},
		{{2, 0}, {3, 1}}
	}));
	ts.append(TM({ // tex_id=1, grass
		{{0, 0}, {1, 1}},
		{{0, 0}, {1, 1}},
		{{0, 0}, {1, 1}},
		{{0, 0}, {1, 1}},
		{{2, 0}, {3, 1}},
		{{1, 0}, {2, 1}}
	}));
	return ts;
}
