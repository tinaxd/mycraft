#include "graphics.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <SOIL/SOIL.h>

using namespace mycraft;

Renderer *mycraft::global_renderer = nullptr;

class mycraft::KeyboardHandlerData
{
public:
	KeyboardHandlerData() {}

private:
	enum class KeyboardButton {
		KEY_W,
		KEY_S,
		KEY_A,
		KEY_D,
		KEY_SHIFT,
		KEY_SPACE
	};

	std::array<bool, 6> button_state_;

	friend void mycraft::keyboard_handler(GLFWwindow *window, int key, int scancode, int action, int mods);
	friend void mycraft::keyboard_handler_tick();
};

Renderer::Renderer(int window_width, int window_height,
		const std::string &window_title)
	: keyboard_handler_data_(new KeyboardHandlerData)
{
	global_renderer = this;

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window_ = glfwCreateWindow(window_width, window_height,
			window_title.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window_);

	glewExperimental = GL_TRUE;
	glewInit();

	prepare_shaders();

	// TODO: when to set callback?
	glfwSetKeyCallback(window_, &keyboard_handler);

	// mouse motion callback
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetCursorPosCallback(window_, &mousemotion_handler);
	glfwGetCursorPos(window_, &last_cursor_xpos, &last_cursor_ypos);
}

struct ShaderCompileError: std::runtime_error
{
	ShaderCompileError(const std::string &what) :
			std::runtime_error(what)
	{
	}
};

GLuint compileShader(const std::string &shader_source, int shader_type,
		const std::string &shader_name)
{
	GLuint shader = glCreateShader(shader_type);
	const char *c_source = shader_source.c_str();
	glShaderSource(shader, 1, &c_source, NULL);
	glCompileShader(shader);

	char buffer[512];
	glGetShaderInfoLog(shader, 512, NULL, buffer);
	std::cout << shader_name << " log:" << std::endl << buffer << std::endl;

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		throw ShaderCompileError("failed to compile " + shader_name);
	}
	else
		std::cout << shader_name << " compilation success" << std::endl;

	return shader;
}

void Renderer::prepare_shaders()
{
	const std::string &vertexSource =
			R"glsl(
#version 150 core

in vec3 block;
in vec2 texCoord;

out vec3 Block;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * model * vec4(block, 1.0);
	Block = block;
	TexCoord = texCoord;
}
)glsl";

	const std::string &fragmentSource =
			R"glsl(
#version 150 core

in vec3 Block;
in vec2 TexCoord;

uniform sampler2D tex0;

out vec4 outColor;

void main()
{
	outColor = texture(tex0, vec2(TexCoord.x / 16.0, 1.0-TexCoord.y/16.0));
	//outColor = texture(tex0, vec2(1.5, 0.5));
	//outColor = vec4(1.0-TexCoord.x/8.0, 1.0, 1.0, 1.0);
}
)glsl";

	// compile shaders
	GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER,
			"vertex shader");
	GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER,
			"fragment shader");
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertexShader);
	glAttachShader(shader_program_, fragmentShader);
	glBindFragDataLocation(shader_program_, 0, "outColor"); // optional
	glLinkProgram(shader_program_);
	glUseProgram(shader_program_);
}

void Renderer::Renderer::render_loop()
{
	// vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// vertex array
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// vertex attribs
	GLint pos_attrib = glGetAttribLocation(shader_program_, "block");
	assert(pos_attrib >= 0);
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 3, GL_BYTE, GL_FALSE, 5 * sizeof(GLbyte),
			0);

	GLint texcoord_attrib = glGetAttribLocation(shader_program_, "texCoord");
	assert(texcoord_attrib >= 0);
	glEnableVertexAttribArray(texcoord_attrib);
	glVertexAttribPointer(texcoord_attrib, 2, GL_BYTE, GL_FALSE,
			5 * sizeof(GLbyte), (void*) (3 * sizeof(GLbyte)));

	// setup uniforms
	model_uni_ = glGetUniformLocation(shader_program_, "model");
	view_uni_ = glGetUniformLocation(shader_program_, "view");
	proj_uni_ = glGetUniformLocation(shader_program_, "proj");

	assert(model_uni_ >= 0);
	assert(view_uni_ >= 0);
	assert(proj_uni_ >= 0);

	// proj
	// TODO: window width, clipping distance
	glm::mat4 proj_mat = glm::perspective(glm::radians(50.0), 800.0 / 600.0,
			1.0, 50.0);
	glUniformMatrix4fv(proj_uni_, 1, GL_FALSE, glm::value_ptr(proj_mat));

	// view initial position
	// TODO: set meaningful initial position
	view_pos_ = glm::vec3(0.0, 0.0, 20);

	// set winding order to Clockwise
	glFrontFace(GL_CW);

	// load textures
	load_textures();

	last_update_time = std::chrono::high_resolution_clock::now();
	while (!glfwWindowShouldClose(window_))
	{
		glfwSwapBuffers(window_);
		glfwPollEvents();

		// player input
		keyboard_handler_tick();

		// draw
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_world();

		last_update_time = std::chrono::high_resolution_clock::now();
	}
}

Renderer::~Renderer()
{
	glfwTerminate();
}

void Renderer::render_world()
{
	if (!world_ || !ts_)
		return;

	// view
	glm::mat4 view_pos_mat = glm::lookAt(view_pos_, view_pos_+view_look_at_vec_, glm::vec3(0, 0, 1));
	glUniformMatrix4fv(view_uni_, 1, GL_FALSE, glm::value_ptr(view_pos_mat));

	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				const auto &chunk = world_->chunk(
				{ i, j, k });
				if (!chunk.has_value())
					continue;

				size_t elements = load_chunk_vertices(*chunk.value());

				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);

				// model
				glm::mat4 model(1.0f);
				constexpr auto cl = Chunk::chunk_length;
				constexpr auto ch = Chunk::chunk_height;
				model = glm::translate(model,
						glm::vec3(cl * i, cl * j, ch * k));
				glUniformMatrix4fv(model_uni_, 1, GL_FALSE,
						glm::value_ptr(model));

				glBindBuffer(GL_ARRAY_BUFFER, vbo);
				//glDrawArrays(GL_LINES, 0, elements);
				glDrawArrays(GL_TRIANGLES, 0, elements);
			}
		}
	}
}

void Renderer::load_textures()
{
	GLuint texture;
	glGenTextures(1, &texture);

	int width, height;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char *image = SOIL_load_image("resources/texture.png", &width,
			&height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
			GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// setup texture uniforms
	GLint tex0Uni = glGetUniformLocation(shader_program_, "tex0");
	assert(tex0Uni >= 0);
	glUniform1i(tex0Uni, 0);
}

enum TexDir
{
	TEX_XNEG, TEX_YNEG, TEX_XPOS, TEX_YPOS, TEX_ZNEG, TEX_ZPOS
};

size_t Renderer::load_chunk_vertices(const Chunk &chunk)
{
	constexpr auto cl = Chunk::chunk_length;
	constexpr auto ch = Chunk::chunk_height;
	constexpr auto elem_count = 5;
	std::array<std::array<GLbyte, elem_count>, cl * cl * ch * 6 * 6> vertices;
	size_t a = 0;

	GLbyte gx, gy, gz;
	auto s =
			[&a, &vertices, &chunk, &gx, &gy, &gz, ts_=this->ts_](GLbyte x,
					GLbyte y, GLbyte z, TexDir tex_dir, bool tex_x, bool tex_y)
					{
						auto& array = vertices[a++];
						array[0] = x; // pos x
					array[1] = y;// pos y
					array[2] = z;// pos z
					const auto blk_id = chunk.data()[Chunk::convert_index(gx, gy, gz)].block_id();
					// TODO: texture id
					const auto& tex_ = ts_->texture(blk_id-1);
					TextureMapCoord tex;
					switch (tex_dir)
					{
						case TEX_XNEG:
						tex = tex_.xneg();
						break;
						case TEX_YNEG:
						tex = tex_.yneg();
						break;
						case TEX_XPOS:
						tex = tex_.xpos();
						break;
						case TEX_YPOS:
						tex = tex_.ypos();
						break;
						case TEX_ZNEG:
						tex = tex_.zneg();
						break;
						case TEX_ZPOS:
						tex = tex_.zpos();
						break;
					}
					array[3] = !tex_x ? tex.first.first : tex.second.first; // tex x
					array[4] = !tex_y ? tex.first.second : tex.second.second;// tex y
				};

	auto exists = [&chunk](GLbyte x, GLbyte y, GLbyte z)
	{
		return chunk.data()[Chunk::convert_index(x, y, z)].block_id() != 0;
	};

	for (int i = 0; i < Chunk::chunk_length; i++)
	{
		for (int j = 0; j < Chunk::chunk_length; j++)
		{
			// flag for z-axis merging
			//std::optional<Block> last_blk = {};
			//std::array<size_t, 6> drawn = {};

			for (int k = 0; k < Chunk::chunk_height; k++)
			{
				const auto &blk = chunk.data()[Chunk::convert_index(i, j, k)];

				if (blk.block_id() == 0)
				{
					continue;
				}

				//const bool can_merge = last_blk.has_value() && last_blk.value().block_id() == blk.block_id();

				gx = i;
				gy = j;
				gz = k;

				//last_blk = std::optional<Block>(blk);

				// view from negative y
//				if (can_merge)
//				{
//					vertices[drawn.at(0)+1][2] = k+1;
//					vertices[drawn.at(0)+2][2] = k+1;
//					vertices[drawn.at(0)+3][2] = k+1;
//				}
//				else
				if (j == 0 || !exists(i, j - 1, k))
				{
					//drawn.at(0) = a;
					s(i, j, k, TEX_YNEG, false, false);
					s(i, j, k + 1, TEX_YNEG, false, true);
					s(i + 1, j, k + 1, TEX_YNEG, true, true);
					s(i + 1, j, k + 1, TEX_YNEG, true, true);
					s(i + 1, j, k, TEX_YNEG, true, false);
					s(i, j, k, TEX_YNEG, false, false);
				}

				// view from positive x
//				if (can_merge)
//				{
//					vertices[drawn.at(1)+1][2] = k+1;
//					vertices[drawn.at(1)+2][2] = k+1;
//					vertices[drawn.at(1)+3][2] = k+1;
//				}
//				else
				if (i == Chunk::chunk_length - 1 || !exists(i + 1, j, k))
				{
					//drawn.at(1) = a;
					s(i + 1, j, k, TEX_XPOS, false, false);
					s(i + 1, j, k + 1, TEX_XPOS, false, true);
					s(i + 1, j + 1, k + 1, TEX_XPOS, true, true);
					s(i + 1, j + 1, k + 1, TEX_XPOS, true, true);
					s(i + 1, j + 1, k, TEX_XPOS, true, false);
					s(i + 1, j, k, TEX_XPOS, false, false);
				}

				// view from positive y
//				if (can_merge)
//				{
//					vertices[drawn.at(2)+1][2] = k+1;
//					vertices[drawn.at(2)+2][2] = k+1;
//					vertices[drawn.at(2)+3][2] = k+1;
//				}
//				else
				if (j == Chunk::chunk_length - 1 || !exists(i, j + 1, k))
				{
					//drawn.at(2) = a;
					s(i + 1, j + 1, k, TEX_YPOS, false, false);
					s(i + 1, j + 1, k + 1, TEX_YPOS, false, true);
					s(i, j + 1, k + 1, TEX_YPOS, true, true);
					s(i, j + 1, k + 1, TEX_YPOS, true, true);
					s(i, j + 1, k, TEX_YPOS, true, false);
					s(i + 1, j + 1, k, TEX_YPOS, false, false);
				}

				// view from negative x
//				if (can_merge)
//				{
//					vertices[drawn.at(3)+1][2] = k+1;
//					vertices[drawn.at(3)+2][2] = k+1;
//					vertices[drawn.at(3)+3][2] = k+1;
//				}
//				else
				if (i == 0 || !exists(i - 1, j, k))
				{
					//drawn.at(3) = a;
					s(i, j + 1, k, TEX_XNEG, false, false);
					s(i, j + 1, k + 1, TEX_XNEG, false, true);
					s(i, j, k + 1, TEX_XNEG, true, true);
					s(i, j, k + 1, TEX_XNEG, true, true);
					s(i, j, k, TEX_XNEG, true, false);
					s(i, j + 1, k, TEX_XNEG, false, false);
				}

				// view from negative z
//				if (can_merge)
//				{}
//				else
				if (k == 0 || !exists(i, j, k - 1))
				{
					//drawn.at(4) = a;
					s(i, j, k, TEX_ZNEG, false, false);
					s(i, j + 1, k, TEX_ZNEG, false, true);
					s(i + 1, j + 1, k, TEX_ZNEG, true, true);
					s(i + 1, j + 1, k, TEX_ZNEG, true, true);
					s(i + 1, j, k, TEX_ZNEG, true, false);
					s(i, j, k, TEX_ZNEG, false, false);
				}

				// view from positive z
//				if (can_merge)
//				{
//					for (int i=0; i<6; i++)
//						vertices[drawn.at(5)+i][2] = k+1;
//				}
//				else
				if (k == Chunk::chunk_height - 1 || !exists(i, j, k + 1))
				{
					//drawn.at(5) = a;
					s(i, j, k + 1, TEX_ZPOS, false, false);
					s(i, j + 1, k + 1, TEX_ZPOS, false, true);
					s(i + 1, j + 1, k + 1, TEX_ZPOS, true, true);
					s(i + 1, j + 1, k + 1, TEX_ZPOS, true, true);
					s(i + 1, j, k + 1, TEX_ZPOS, true, false);
					s(i, j, k + 1, TEX_ZPOS, false, false);
				}
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, elem_count * a * sizeof(GLbyte),
			vertices.data(),
			GL_STATIC_DRAW);
	chunk.set_changed(false);
	return elem_count * a;
}

void mycraft::keyboard_handler(GLFWwindow *window, int key, int scancode,
		int action, int mods)
{
	Renderer *self = global_renderer;
	auto& data = self->keyboard_handler_data_;

	auto& state = data->button_state_;

	auto ed = [&state, &action, &key](int key_target, KeyboardHandlerData::KeyboardButton btn) {
		if (key == key_target) {
			if (action == GLFW_PRESS)
				state[static_cast<size_t>(btn)] = true;
			else if (action == GLFW_RELEASE)
				state[static_cast<size_t>(btn)] = false;
		}
	};

	using KB = KeyboardHandlerData::KeyboardButton;
	ed(GLFW_KEY_W, KB::KEY_W);
	ed(GLFW_KEY_S, KB::KEY_S);
	ed(GLFW_KEY_A, KB::KEY_A);
	ed(GLFW_KEY_D, KB::KEY_D);
	ed(GLFW_KEY_SPACE, KB::KEY_SPACE);
	ed(GLFW_KEY_LEFT_SHIFT, KB::KEY_SHIFT);
	// TODO: GLFW_KEY_RIGHT_SHIFT?
}

void mycraft::keyboard_handler_tick()
{
	Renderer *self = global_renderer;
	const auto& lookat_vec = self->view_look_at_vec_;
	const auto& speed = self->walk_speed;
	const auto& state = self->keyboard_handler_data_->button_state_;
	using KB = KeyboardHandlerData::KeyboardButton;
	auto& view_pos = self->view_pos_;
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(
			std::chrono::high_resolution_clock::now() - self->last_update_time).count();

	const glm::vec2 forward = glm::normalize(glm::vec2(lookat_vec.x, lookat_vec.y)) * speed * dt;
	glm::vec3 vel(0.0, 0.0, 0.0);
	if (state[static_cast<size_t>(KB::KEY_W)])
	{
		vel.x += forward.x;
		vel.y += forward.y;
	}
	if (state[static_cast<size_t>(KB::KEY_S)])
	{
		vel.x -= forward.x;
		vel.y -= forward.y;
	}
	if (state[static_cast<size_t>(KB::KEY_A)])
	{
		vel.x -= forward.y;
		vel.y += forward.x;
	}
	if (state[static_cast<size_t>(KB::KEY_D)])
	{
		vel.x += forward.y;
		vel.y -= forward.x;
	}
	if (state[static_cast<size_t>(KB::KEY_SPACE)])
	{
		vel.z += speed*dt;
	}
	if (state[static_cast<size_t>(KB::KEY_SHIFT)])
	{
		vel.z -= speed*dt;
	}

	view_pos += vel;
}

void mycraft::mousemotion_handler(GLFWwindow *window, double xpos, double ypos)
{
	Renderer *self = global_renderer;

	double dx = xpos - self->last_cursor_xpos;
	double dy = ypos - self->last_cursor_ypos;
	self->last_cursor_xpos = xpos;
	self->last_cursor_ypos = ypos;

	// TODO: user settings
	float dx_speed = -0.0005f;
	float dy_speed = -0.0005f;

	auto& ax = self->camera_angles_x;
	auto& ay = self->camera_angles_y;

	ax += dx * dx_speed;
	if (ax < -M_PI)
		ax += 2*M_PI;
	else if (ax > M_PI)
		ax -= 2*M_PI;

	ay += dy * dy_speed;
	if (ay < -M_PI/2)
		ay = -M_PI/2;
	else if (ay > M_PI/2)
		ay = M_PI/2;

	auto& lookat = self->view_look_at_vec_;
	lookat.x = std::cos(ax) * std::cos(ay);
	lookat.y = std::sin(ax) * std::cos(ay);
	lookat.z = std::sin(ay);
}
