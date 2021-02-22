#include "graphics.h"

#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cassert>

using namespace mycraft;

Renderer::Renderer(int window_width, int window_height,
		const std::string &window_title)
{
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

in vec4 block;

out vec4 Block;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * model * vec4(block.xyz, 1.0);
	Block = block;
}
)glsl";

	const std::string &fragmentSource =
			R"glsl(
#version 150 core

in vec4 Block;

out vec4 outColor;

void main()
{
	float blk = Block.w;
	outColor = vec4(blk/128.0, blk/256.0, blk/512.0, 1.0);
	//outColor = vec4(1.0, 1.0, 1.0, 1.0);
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
	glEnableVertexAttribArray(pos_attrib);
	glVertexAttribPointer(pos_attrib, 4, GL_BYTE, GL_FALSE, 0, 0);

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

	// set winding order to Clockwise
	glFrontFace(GL_CW);

	while (!glfwWindowShouldClose(window_))
	{
		glfwSwapBuffers(window_);
		glfwPollEvents();

		// draw
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//render_world(0, 0, 0); // TODO: input camera's coord
		render_world(-10, -15, 20);
	}
}

Renderer::~Renderer()
{
	glfwTerminate();
}

void Renderer::render_world(CoordElem pos_x, CoordElem pos_y, CoordElem pos_z)
{
	if (!world_)
		return;

	// view
	glm::mat4 view_mat = glm::lookAt(glm::vec3(pos_x, pos_y, pos_z),
			glm::vec3(0, 0, 20), glm::vec3(0, 0, 1));
	//glm::mat4 view_mat(1.0f);
	glUniformMatrix4fv(view_uni_, 1, GL_FALSE, glm::value_ptr(view_mat));

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
				glDrawArrays(GL_TRIANGLES, 0, elements);
			}
		}
	}
}

size_t Renderer::load_chunk_vertices(const Chunk &chunk)
{
	constexpr auto cl = Chunk::chunk_length;
	constexpr auto ch = Chunk::chunk_height;
	std::array<std::array<GLbyte, 4>, cl * cl * ch * 6 * 6> vertices;
	size_t a = 0;

	GLbyte gx, gy, gz;
	auto s =
			[&a, &vertices, &chunk, &gx, &gy, &gz](GLbyte x, GLbyte y,
					GLbyte z)
					{
						auto& array = vertices[a++];
						array[0] = x;
						array[1] = y;
						array[2] = z;
						array[3] = chunk.data()[Chunk::convert_index(gx, gy, gz)].block_id();
					};

	auto exists = [&chunk](GLbyte x, GLbyte y, GLbyte z)
	{
		return chunk.data()[Chunk::convert_index(x, y, z)].block_id() != 0;
	};

	for (int i = 0; i < Chunk::chunk_length; i++)
	{
		for (int j = 0; j < Chunk::chunk_length; j++)
		{
			for (int k = 0; k < Chunk::chunk_height; k++)
			{
				if (!exists(i, j, k))
				{
					continue;
				}

				gx = i;
				gy = j;
				gz = k;

				// view from negative y
				if (j == 0 || !exists(i, j - 1, k))
				{
					s(i, j, k);
					s(i, j, k + 1);
					s(i + 1, j, k + 1);
					s(i + 1, j, k + 1);
					s(i + 1, j, k);
					s(i, j, k);
				}

				// view from positive x
				if (i == Chunk::chunk_length - 1 || !exists(i + 1, j, k))
				{
					s(i + 1, j, k);
					s(i + 1, j, k + 1);
					s(i + 1, j + 1, k + 1);
					s(i + 1, j + 1, k + 1);
					s(i + 1, j + 1, k);
					s(i + 1, j, k);
				}

				// view from positive y
				if (j == Chunk::chunk_length - 1 || !exists(i, j + 1, k))
				{
					s(i + 1, j + 1, k);
					s(i + 1, j + 1, k + 1);
					s(i, j + 1, k + 1);
					s(i, j + 1, k + 1);
					s(i, j + 1, k);
					s(i + 1, j + 1, k);
				}

				// view from negative x
				if (i == 0 || !exists(i - 1, j, k))
				{
					s(i, j + 1, k);
					s(i, j + 1, k + 1);
					s(i, j, k + 1);
					s(i, j, k + 1);
					s(i, j, k);
					s(i, j + 1, k);
				}

				// view from negative z
				if (k == 0 || !exists(i, j, k - 1))
				{
					s(i, j, k);
					s(i, j + 1, k);
					s(i + 1, j + 1, k);
					s(i + 1, j + 1, k);
					s(i + 1, j, k);
					s(i, j, k);
				}

				// view from positive z
				if (k == Chunk::chunk_height - 1 || !exists(i, j, k + 1))
				{
					s(i, j, k + 1);
					s(i, j + 1, k + 1);
					s(i + 1, j + 1, k + 1);
					s(i + 1, j + 1, k + 1);
					s(i + 1, j, k + 1);
					s(i, j, k + 1);
				}
			}
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * a * sizeof(GLbyte), vertices.data(),
			GL_STATIC_DRAW);
	chunk.set_changed(false);
	return 4 * a;
}
