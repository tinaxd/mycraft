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
#include <chrono>

namespace mycraft
{
	class KeyboardHandlerData;
	void keyboard_handler(GLFWwindow *window, int key, int scancode, int action, int mods);
	void keyboard_handler_tick();
	void mousemotion_handler(GLFWwindow *window, double xpos, double ypos);

	// TODO: this class should be a singleton.
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
		// World data
		std::shared_ptr<World> world_;

		// Texture data
		std::shared_ptr<TextureStorage> ts_;

		// OpenGL
		GLuint shader_program_;
		GLuint vbo;

		float walk_speed = 100.0;
		// player's position
		glm::vec3 view_pos_;
		glm::vec3 view_look_at_vec_;
		GLint view_uni_;

		GLint proj_uni_;

		GLint model_uni_;

		// time
		std::chrono::high_resolution_clock::time_point last_update_time;

		// keyboard input
		std::unique_ptr<KeyboardHandlerData> keyboard_handler_data_;

		// mouse input
		double last_cursor_xpos, last_cursor_ypos;
		float camera_angles_x = .0f;
		float camera_angles_y = .0f;

		void prepare_shaders();
		void load_textures();
		void render_world();

		size_t load_chunk_vertices(const Chunk& chunk);

		friend void keyboard_handler(GLFWwindow *window, int key, int scancode, int action, int mods);
		friend void mousemotion_handler(GLFWwindow *window, double xpos, double ypos);
		friend void keyboard_handler_tick();
	};

	extern Renderer *global_renderer;
}
