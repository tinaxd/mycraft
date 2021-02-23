#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <thread>
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "world.h"
#include "TextureMap.h"

namespace mycraft
{
	class Renderer
	{
	public:
		Renderer(int window_width, int window_height,
				const std::string &window_title);
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;

		~Renderer();

		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void render_loop();

		void set_world(std::shared_ptr<World> world)
		{
			world_ = std::move(world);
		}

		void set_texture_storage(std::shared_ptr<TextureStorage> ts)
		{
			ts_ = std::move(ts);
		}

		const std::shared_ptr<World>& get_world() const
		{
			return world_;
		}

	private:
		GLFWwindow *window_;
		std::shared_ptr<World> world_;
		std::shared_ptr<TextureStorage> ts_;
		GLuint shader_program_;
		GLuint vbo;

		GLint view_uni_;
		GLint proj_uni_;

		GLint model_uni_;

		void prepare_shaders();
		void load_textures();
		void render_world(CoordElem pos_x, CoordElem pos_y, CoordElem pos_z);

		size_t load_chunk_vertices(const Chunk& chunk);
	};
}
