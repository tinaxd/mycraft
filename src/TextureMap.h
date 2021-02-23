/*
 * TextureMap.h
 *
 *  Created on: 2021/02/23
 *      Author: tinaxd
 */

#ifndef TEXTUREMAP_H_
#define TEXTUREMAP_H_

#include <vector>
#include <utility>
#include <cstdint>
#include <vector>
#include "block.h"

namespace mycraft
{

using TextureCoord = std::pair<std::int8_t, std::int8_t>;
using TextureMapCoord = std::pair<TextureCoord, TextureCoord>;

using texture_id_t = size_t;

class TextureStorage
{
public:
	class TextureMap
	{
	public:
		TextureMap(const std::vector<TextureMapCoord>& coords);

		const TextureMapCoord& zpos() const {return zpos_;}
		const TextureMapCoord& zneg() const {return zneg_;}
		const TextureMapCoord& xneg() const {return xneg_;}
		const TextureMapCoord& xpos() const {return xpos_;}
		const TextureMapCoord& yneg() const {return yneg_;}
		const TextureMapCoord& ypos() const {return ypos_;}

	private:
		TextureMapCoord xneg_;
		TextureMapCoord yneg_;
		TextureMapCoord xpos_;
		TextureMapCoord ypos_;
		TextureMapCoord zneg_;
		TextureMapCoord zpos_;
	};

	 texture_id_t append(TextureMap map);
	 const TextureMap& texture(texture_id_t tex_id) {return textures_.at(tex_id);}

private:
	std::vector<TextureMap> textures_;
};

TextureStorage standard_texture_storage();

}

#endif /* TEXTUREMAP_H_ */
