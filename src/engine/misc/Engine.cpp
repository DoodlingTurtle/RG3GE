#include "../Engine.h"
#include "../../engine_config.h"
#include "../Macros.h"

#include "../Transform.h"
#include <iostream>
#include <algorithm>

#include "../vendor/stb_image.h"

#ifdef DEBUG_BUILD
void CheckOGLErr(const char* stmt, const char* fname, int line) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		std::cout << "OpenGL error " << err << " at " << fname << ":" << line << " => " << stmt << std::endl;
	}
}
#define GLCALL(stmt) do {\
  stmt; \
  CheckOGLErr(#stmt, __FILE__, __LINE__); \
} while(0)
#else
#define GLCALL(s) s
#endif

namespace RG3GE {
#pragma region TextureSlots
	struct TextureSlot {
		int width = 0, height = 0, colorchannels = 0;
		unsigned int _gl_texture_id = -1;
		unsigned int users = 0;
	};
	static TextureSlot  _texture_slots[ENGINE_TEXTURE_LIMIT];
	static void freeTextureSlot(unsigned int slot, bool ignoreUsers = false) {
		if (_texture_slots[slot].users > 0) _texture_slots[slot].users--;

		if (ignoreUsers || _texture_slots[slot].users == 0) {
			GLCALL(glDeleteTextures(1, &(_texture_slots[slot]._gl_texture_id)));
			_texture_slots[slot].width = 0;
			_texture_slots[slot].height = 0;
			_texture_slots[slot].colorchannels = 0;
		}
	}
	static int nextFreeTextureSlot() {
		for (int a = 0; a < ENGINE_TEXTURE_LIMIT; a++) {
			if (_texture_slots[a].colorchannels == 0)
				return a;
		}
		return -1;
	}
#pragma endregion

#pragma region Renderer

        void Engine::resizeWindow(int newWidth, int newHeight) {
            if(newWidth > 0 && newHeight > 0) {
                SDL_SetWindowSize(window, newWidth, newHeight);
            }
        }


	union RenderSubject {
		Shape2D shape;
		Texture texture;

		RenderSubject() {};
	};

	struct RenderJob {
		Transform tr;
		unsigned char type;
		float zDepth;
		RenderSubject subject;
		Color tint;

		RenderJob() : tr(), type(0), zDepth(0), subject(), tint(0.0f, 0.0f, 0.0f, 0.0f) {}
		RenderJob(Transform tr, unsigned char type, float zDepth, Shape2D shape, Color c)
			: tr(tr), type(type), zDepth(zDepth), tint(c)
		{
			subject.shape = shape;
		}

		RenderJob(Transform tr, unsigned char type, float zDepth, Texture texture, Color c)
			: tr(tr), type(type), zDepth(zDepth), tint(c)
		{
			subject.texture = texture;
		}
	};
	static std::vector<RenderJob> _render_jobs;

	void Engine::SubmitForRender(Shape2D& shape, Transform& tr, float zDepth) {
		_render_jobs.push_back({ tr, 0, zDepth, shape, currentTint });
	}
	void Engine::SubmitForRender(Texture& texture, Transform& tr, float zDepth) {
		_render_jobs.push_back({ tr, 1, zDepth, texture, currentTint });
	}

	static bool _rendersort(RenderJob& a, RenderJob& b) { return a.zDepth > b.zDepth; }
	void Engine::RenderAll() {
		std::sort(_render_jobs.begin(), _render_jobs.end(), _rendersort);

		Color c = Color(0.0f, 0.0f, 0.0f, 0.0f);
		for (auto j : _render_jobs) {
			if (c != j.tint) {
				c = j.tint;
				glUniform4f(_gl_drawcolor_uniform, c.r, c.g, c.b, c.a);
			}
			switch (j.type) {
			case 0: DrawShape2D(j.subject.shape, j.tr, j.zDepth); break;
			case 1: TextureDraw(j.subject.texture, j.tr, j.zDepth); break;
			}
		}

		_render_jobs.clear();
		SDL_GL_SwapWindow(window);
	}
#pragma endregion

#pragma region RG3GE::Vec2
	std::ostream& operator << (std::ostream& os, const Vec2<int>& v) { os << " Vec2<int>(" << v.x << ", " << v.y << ")"; return os; }
	std::ostream& operator << (std::ostream& os, const Vec2<float>& v) { os << " Vec2<float>(" << v.x << ", " << v.y << ")"; return os; }
	std::ostream& operator << (std::ostream& os, const Vec2<double>& v) { os << " Vec2<double>(" << v.x << ", " << v.y << ")"; return os; }
	std::ostream& operator << (std::ostream& os, const Vec2<short>& v) { os << " Vec2<short>(" << v.x << ", " << v.y << ")"; return os; }
#pragma endregion

#pragma region RG3GE::Angle
	Angle::Angle(double ang) {
		angle = ang * PI2 / 360.0;
		direction.x = cos(angle);
		direction.y = sin(angle);
	}
	Angle::Angle(double dirx, double diry) {
		direction.x = dirx;
		direction.y = diry;
		angle = acos(diry);
	}
	Angle& Angle::operator += (double a) {
		angle += a / 360.0f * PI2;
		direction.x = cos(angle);
		direction.y = sin(angle);
		return *this;
	}
	Angle& Angle::operator -= (double a) {
		angle -= a / 360.0f * PI2;
		direction.x = cos(angle);
		direction.y = sin(angle);
		return *this;
	}
#pragma endregion

#pragma region RG3GE::Color
	/*==============================================================================
	 * RG3GE::Color
	 *============================================================================*/
	Color::Color(int r, int g, int b, int a)
		: r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f)
	{}
	Color::Color(float r, float g, float b, float a)
		: r(r), g(g), b(b), a(a)
	{}
	bool Color::operator != (Color& c) { return (c.r != r) || (c.g != g) || (c.b != b) || (c.a != a); }
#pragma endregion

#pragma region RG3GE::Vertex2D
	Vertex2D::Vertex2D()
		: position(0, 0)
		, vertexColor(1.0f, 1.0f, 1.0f, 1.0f)
		, uvCoords(0.0f, 0.0f)
	{}
	Vertex2D::Vertex2D(float x, float y)
		: position((float)x, (float)y)
		, vertexColor(1.0f, 1.0f, 1.0f, 1.0f)
		, uvCoords(0.0f, 0.0f)
	{}
	Vertex2D::Vertex2D(float x, float y, int r, int g, int b, int a)
		: position(x, y), vertexColor(r, g, b, a)
		, uvCoords(0.0f, 0.0f)
	{}
	Vertex2D::Vertex2D(float x, float y, Color c)
		: position(x, y), vertexColor(c)
		, uvCoords(0.0f, 0.0f)
	{}
	Vertex2D::Vertex2D(float x, float y, float uvx, float uvy)
		: position(x, y), vertexColor(1.0f, 1.0f, 1.0f, 1.0f)
		, uvCoords(uvx, uvy)
	{}
#pragma endregion

#pragma region RG3GE::Shape2D
	Shape2D::Shape2D()
		: shape(PolyShapes::POINTS)
		, vertexCnt(0), vertexBuffer(0)
	{}
#pragma endregion

#pragma region RG3GE::Engine
	static unsigned int CompileShader(unsigned int type, std::string& src) {
		unsigned int shader = glCreateShader(type);
		const char* sr = src.c_str();
		int l = (int)src.length();
		glShaderSource(shader, 1, &sr, &l);
		glCompileShader(shader);

		int state;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &state);

		if (state != 1) {
			std::cout << "Could not comile shader type " << src << std::endl;
#ifdef DEBUG_BUILD
			int log_size = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
			char* buffer = (char*)alloca(log_size);
			glGetShaderInfoLog(shader, log_size, &log_size, buffer);
			Debug(buffer);
#endif 
		}

		return shader;

	}

	const Color Engine::WHITE = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color Engine::BLACK = Color(0.0f, 0.0f, 0.0f, 1.0f);

	Engine* Engine::_instance = nullptr;

	Engine* Engine::init(int winWidth, int winHeight, const char* winTitle, unsigned int sdl_init_flags, std::function<bool(const Engine*)> onBuild) {
		if (_instance) {
			std::cout << "Engine::init can only run once per Application start" << std::endl
				<< "If the previous Engine has stopped, please restart the Application" << std::endl;
			return nullptr;
		}

		if (SDL_Init(sdl_init_flags | SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
			std::cout << SDL_GetError() << std::endl;
			return nullptr;
		}

		SDL_SetHintWithPriority(SDL_HINT_RENDER_VSYNC, "0", SDL_HINT_OVERRIDE);

		SDL_ShowCursor(SDL_DISABLE);

		Engine* e = new Engine();
		_instance = e;

		e->window = SDL_CreateWindow(winTitle,
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			winWidth, winHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (!e->window) {
			std::cout << "could not create window: " << SDL_GetError() << std::endl;
			return nullptr;
		}

		e->context = SDL_GL_CreateContext(e->window);
		if (!e->context) {
			std::cout << "could not create window: " << SDL_GetError() << std::endl;
			return nullptr;
		}

		if (glewInit() != GLEW_OK) {
			std::cout << "failed to setup glew " << std::endl;
			return nullptr;
		}

		if (SDL_GL_SetSwapInterval(1) < 0) {
			std::cout << "failed to setup vsync " << SDL_GetError() << std::endl;
			e->keepRunning = false;
			return nullptr;
		};

		GLCALL(glLoadIdentity());
		GLCALL(glOrtho(0, winWidth, winHeight, 0, -1, 1));
		GLCALL(glEnable(GL_DEPTH_TEST));

		// Alpha Blending 
		GLCALL(glEnable(GL_BLEND));
		GLCALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		e->windowSize = (Vec2<float>)Vec2<int>(winWidth, winHeight);
		e->origWindowSize = e->windowSize;

		_render_jobs.reserve(ENGINE_DRAW_CALL_LIMIT);

		//Init OpenGLShader
		std::string vs =
			"#version 330 core\n"
			"\n"
			"layout(location = 0) in vec2 position;\n"
			"layout(location = 1) in vec4 vertCol;\n"
			"layout(location = 2) in vec2 uvCoords;\n"
			"\n"
			"uniform vec2 translation;\n"
			"uniform vec2 origin;\n"
			"uniform float zlayer;\n"
			"uniform vec2 angle;\n"
			"uniform vec2 scale;\n"
			"\n"
			"uniform vec2 v2screen;\n"
			"uniform vec2 v2screenscale;\n"
			"uniform vec2 v2screenoffset;\n"
			"\n"
			"out vec4 vertcolor;\n"
			"out vec2 uvs;\n"
			"\n"
			"void main() {\n"
			"   vertcolor = vertCol;\n"
			"   uvs       = uvCoords;\n"
			"\n"
			"   vec2 finalOrig = position - origin;\n"
			"        finalOrig *= (scale*v2screenscale);\n"
			"\n"
			"   vec2 finalPos = vec2("
			"                     finalOrig.x * angle.x + finalOrig.y * (-angle.y),"
			"                     finalOrig.x * angle.y + finalOrig.y *  angle.x"
			"                   );\n"
			"        finalPos += (translation*v2screenscale) + v2screenoffset;\n"
			"\n"
			"   gl_Position = vec4(finalPos / v2screen"
			"                - vec2(1, -1)"
			"   ,zlayer , 1);\n"
			"}\n";

		std::string fs =
			"#version 330 core\n"
			"\n"
			"out vec4 color;\n"
			"\n"
			"uniform vec4 drawcolor;\n"
			"uniform sampler2D mytexture;\n"
			"uniform float texture_enable;\n"
			"\n"
			"in vec4 vertcolor;\n"
			"in vec2 uvs;\n"
			"\n"
			"void main() {\n"
			"	color = "
			"      ((texture_enable * texture(mytexture, uvs)) "
			"       + ((1.0 - texture_enable) * vec4(1.0, 1.0, 1.0, 1.0)))"
			"      * drawcolor * vertcolor;\n"
			"}\n";

		unsigned int iVS = CompileShader(GL_VERTEX_SHADER, vs);
		unsigned int iFS = CompileShader(GL_FRAGMENT_SHADER, fs);
		unsigned int program = glCreateProgram();

		glAttachShader(program, iVS);
		glAttachShader(program, iFS);
		glLinkProgram(program);

		int link_state;
		glGetProgramiv(program, GL_LINK_STATUS, &link_state);
		glDeleteShader(iVS);
		glDeleteShader(iFS);

		if (link_state != 1) {
			std::cout << "Could not link program type " << std::endl;
#ifdef DEBUG_BUILD
			int log_size = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
			char* buffer = (char*)alloca(log_size);
			glGetProgramInfoLog(program, log_size, &log_size, buffer);
			std::cout << buffer << std::endl;
#endif 
			return nullptr;
		}

		glUseProgram(program);

		GLCALL(e->_gl_texture_enable_uniform = glGetUniformLocation(program, "texture_enable"));
		GLCALL(e->_gl_translation_uniform = glGetUniformLocation(program, "translation"));
		GLCALL(e->_gl_origin_uniform = glGetUniformLocation(program, "origin"));
		GLCALL(e->_gl_zlayer_uniform = glGetUniformLocation(program, "zlayer"));
		GLCALL(e->_gl_angle_uniform = glGetUniformLocation(program, "angle"));
		GLCALL(e->_gl_drawcolor_uniform = glGetUniformLocation(program, "drawcolor"));
		GLCALL(e->_gl_scale_uniform = glGetUniformLocation(program, "scale"));
		GLCALL(e->_gl_v2screen_uniform = glGetUniformLocation(program, "v2screen"));
		GLCALL(e->_gl_v2screenscale_uniform = glGetUniformLocation(program, "v2screenscale"));
		GLCALL(e->_gl_v2screenoffset_uniform = glGetUniformLocation(program, "v2screenoffset"));
		GLCALL(e->_gl_texture_uniform = glGetUniformLocation(program, "mytexture"));

		GLCALL(glEnableVertexAttribArray(e->_gl_vertex_position_attribute));
		GLCALL(glEnableVertexAttribArray(e->_gl_vertex_color_attribute));
		GLCALL(glEnableVertexAttribArray(e->_gl_vertexUV_attribute));

		e->_applyScreenSize();

		Vertex2D pixeldata[] = {
		  {0.0f, 0.0f, 0.0f, 0.0f},
		  {1.0f, 0.0f, 1.0f, 0.0f},
		  {1.0f, 1.0f, 1.0f, 1.0f},
		  {0.0f, 1.0f, 0.0f, 1.0f}
		};
		e->pixel = e->CreateShape2D(PolyShapes::QUADS, 4, pixeldata);

		Vertex2D linedata[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f } };
		e->line = e->CreateShape2D(PolyShapes::LINES, 2, linedata);

		if (onBuild(e)) {
			e->keepRunning = true;
			return e;
		}

		return nullptr;
	}

	void Engine::cleanup() {

		// Free all texture Slots
		for (int a = 0; a < ENGINE_TEXTURE_LIMIT; a++) {
			if (_texture_slots[a].colorchannels > 0) {
				freeTextureSlot(a, true);
			}
		}

		//TODO: Destroy all left Shape2Ds
		if (_instance)
			delete _instance;

		SDL_Quit();
	}

	Engine::Engine()
		: borderColor(Engine::BLACK)
		, currentTint(1.0f, 1.0f, 1.0f, 1.0f)
		, _deltaTime(0.0f)
		, windowSize(0)
		, origWindowSize(0)
		, windowOffset(0)
		, windowScale(0)
		, ticks(0)
		, _gl_vertex_position_attribute(0)
		, _gl_vertex_color_attribute(1)
		, _gl_vertexUV_attribute(2)
		, _gl_texture_enable_uniform(-1)
		, _gl_texture_uniform(-1)
		, _gl_translation_uniform(-1)
		, _gl_origin_uniform(-1)
		, _gl_zlayer_uniform(-1)
		, _gl_angle_uniform(-1)
		, _gl_drawcolor_uniform(-1)
		, _gl_scale_uniform(-1)
		, _gl_v2screen_uniform(-1)
		, _gl_v2screenscale_uniform(-1)
		, _gl_v2screenoffset_uniform(-1)
	{}

	Engine::~Engine() {

		DestroyShape2D(pixel);

		GLCALL(glDisableVertexAttribArray(_gl_vertexUV_attribute));
		GLCALL(glDisableVertexAttribArray(_gl_vertex_position_attribute));
		GLCALL(glDisableVertexAttribArray(_gl_vertex_color_attribute));

		if (context) SDL_GL_DeleteContext(context);
		if (window)	SDL_DestroyWindow(window);
	}

	bool Engine::windowTick() {
		Uint32 currentTicks = SDL_GetTicks();
		Uint32 deltaTicks = currentTicks - ticks;
		_deltaTime = (float)deltaTicks / 1000.0f;

		if (_deltaTime > 0) {

			for (auto it : keys_pressed)
				keys_held.emplace(it);
			
			for (auto it : mouse_pressed)
				mouse_held.emplace(it);

			keys_pressed.clear();
			keys_released.clear();
			mouse_pressed.clear();
			mouse_released.clear();

			while (keepRunning && SDL_PollEvent(&event)) {
				switch (event.type) {
					// TODO: Process other events
				case SDL_QUIT: keepRunning = false; break;

				case SDL_MOUSEBUTTONDOWN: {
					mouse_pressed.emplace(event.button.button);
				} break;

				case SDL_MOUSEBUTTONUP: {
					mouse_released.emplace(event.button.button);
					auto it = mouse_held.find(event.key.keysym.sym);
					if (it != mouse_held.end()) mouse_held.erase(it);
				} break;

				case SDL_MOUSEMOTION: {
					mousePosition.x = (float)event.motion.x;
					mousePosition.y = (float)event.motion.y;
					mousePosition -= windowOffset / 2;
					mousePosition /= windowScale;
				}  break;

				case SDL_KEYDOWN: {
					if (event.key.keysym.sym != last_pressed)
						keys_pressed.emplace(event.key.keysym.sym);

					last_pressed = event.key.keysym.sym;
				} break;

				case SDL_KEYUP: {
					keys_released.emplace(event.key.keysym.sym);
					auto it = keys_held.find(event.key.keysym.sym);
					if (it != keys_held.end()) keys_held.erase(it);

					if (event.key.keysym.sym == last_pressed) last_pressed = 0;
				} break;

				case SDL_WINDOWEVENT:
					switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						windowSize = (Vec2<float>)Vec2<int>(event.window.data1, event.window.data2);
						_applyScreenSize();
						break;
					}
					break;
				}
			};

			if (keepRunning) {
				//TODO: Update World
				//TODO: Update Scene
			}

			if (keepRunning) {
				//TODO: Update Viewport

				// Draw some nice bars, if window aspect does not fit viewport aspect
				SetTint(borderColor);
				//TODO: Move to _applyScreenSize
				if (windowOffset.x > 0 || windowOffset.y > 0) {
					if (windowOffset.x > windowOffset.y) {
						Transform tr = {
						  {0, 0}, {0, 0},
						  {-(windowOffset.x / windowScale.x), windowSize.y / windowScale.y}, 0.0f
						};
						SubmitForRender(pixel, tr, -1);

						tr.position.x += origWindowSize.x;
						tr.scale.x *= -1;
						SubmitForRender(pixel, tr, -1);
					}
					else {
						Transform tr = {
						  {0, 0}, {0, 0},
						  {windowSize.x / windowScale.x,-(windowOffset.y / windowScale.y)}, 0.0f
						};
						SubmitForRender(pixel, tr, -1);

						tr.position.y += origWindowSize.y;
						tr.scale.y *= -1;
						SubmitForRender(pixel, tr, -1);
					}
				}
				SetTint(WHITE);

				RenderAll();
			}

			ticks = currentTicks;
		}

		return keepRunning;
	}

	float Engine::deltaTime() {
		return _deltaTime;
	}

	void Engine::SetTint(Color c) {
		currentTint = c;
	}

	void Engine::ClearScreen(Color c) {
		glClearColor(c.r, c.g, c.b, c.a);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	}

	void Engine::_applyScreenSize() {
		GLCALL(glUniform2f(_gl_v2screen_uniform, (float)windowSize.x, (float)-windowSize.y));

		Vec2<float> scale = (Vec2<float>)windowSize / (Vec2<float>)origWindowSize;
		if (scale.x > scale.y) scale.x = scale.y;
		else                  scale.y = scale.x;

		windowScale = scale;
		GLCALL(glUniform2f(_gl_v2screenscale_uniform, scale.x, scale.y));

		Vec2<float> offset = origWindowSize * scale;
		offset = windowSize - offset;

		GLCALL(glUniform2f(_gl_v2screenoffset_uniform, offset.x, offset.y));
		windowOffset = offset;

		GLCALL(glViewport(0, 0, (int)windowSize.x, (int)windowSize.y));
	}

	void Engine::_applyTransform(Transform& transform, float zLayer) {
		Vec2<float> rot = (Vec2<float>)transform.rotation.direction;
		GLCALL(glUniform2f(_gl_translation_uniform, transform.position.x * 2, transform.position.y * 2));
		GLCALL(glUniform2f(_gl_origin_uniform, transform.origin.x, transform.origin.y));
		GLCALL(glUniform1f(_gl_zlayer_uniform, zLayer));
		GLCALL(glUniform2f(_gl_angle_uniform, rot.x, rot.y));
		GLCALL(glUniform2f(_gl_scale_uniform, transform.scale.x * 2, transform.scale.y * 2));
	}
#pragma endregion

#pragma region RG3GE::Engine::Input - Functions

	bool Engine::keyPressed(SDL_Keycode code) { return keys_pressed.find(code) != keys_pressed.end(); }
	bool Engine::keyReleased(SDL_Keycode code) { return keys_released.find(code) != keys_released.end(); }
	bool Engine::keyHeld(SDL_Keycode code) { return keys_held.find(code) != keys_held.end(); }

	bool Engine::mousePressed(Uint8 code) { return mouse_pressed.find(code)  != mouse_pressed.end();  }
	bool Engine::mouseHeld(Uint8 code)    { return mouse_held.find(code)     != mouse_held.end();  }
	bool Engine::mouseReleased(Uint8 code){ return mouse_released.find(code) != mouse_released.end();  }

#pragma endregion

#pragma region RG3GE::Engine::Shape2D - Functions
	Shape2D Engine::CreateShape2D(RG3GE::PolyShapes shape, const std::vector<Vertex2D>& data) {
		Vertex2D* points = (Vertex2D*)alloca(sizeof(Vertex2D) * data.size());

		int cnt = 0;
		for (auto p : data) {
			points[cnt] = p;
			cnt++;
		}

		return CreateShape2D(shape, cnt, points);
	}

	Shape2D Engine::CreateShape2D(RG3GE::PolyShapes shape, int verts, const Vertex2D points[]) {
		Shape2D ret;
		ret.vertexCnt = verts;
		ret.shape = shape;

		// TODO: Keep track of all created buffers inside Engine::_instance (then clean out on delete)
		GLCALL(glGenBuffers(1, &ret.vertexBuffer));
		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, ret.vertexBuffer));
		GLCALL(glBufferData(GL_ARRAY_BUFFER, verts * sizeof(Vertex2D), points, GL_DYNAMIC_DRAW));

		return ret;
	};

	void Engine::DestroyShape2D(Shape2D s) {
		GLCALL(glDeleteBuffers(1, &s.vertexBuffer));
		s.vertexBuffer = 0;
	}

	void Engine::DrawShape2D(Shape2D shape, Transform& tr, float zLayer) {
		glEnableClientState(GL_VERTEX_ARRAY);
		glPushMatrix();

		_applyTransform(tr, zLayer);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vertexBuffer);
		glVertexPointer(2, GL_FLOAT, 0, NULL);

		GLCALL(glVertexAttribPointer(_gl_vertex_position_attribute, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), 0));
		GLCALL(glVertexAttribPointer(_gl_vertex_color_attribute, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), (void*)(2 * sizeof(GL_FLOAT))));
		GLCALL(glVertexAttribPointer(_gl_vertexUV_attribute, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), (void*)(6 * sizeof(GL_FLOAT))));

		glDrawArrays(static_cast<GLint>(shape.shape), 0, shape.vertexCnt);

		glPopMatrix();
		glDisableClientState(GL_VERTEX_ARRAY);
	}
#pragma endregion

#pragma region RG3GE::Engine::Draw... - Functions
	void Engine::DrawRectFilled(int x, int y, int w, int h, Color c, float zLayer) {
		Color t = currentTint;

		currentTint = c;
		Transform tr = {
		  (Vec2<float>)Vec2<int>(x, y),
		  { 0 },
		  (Vec2<float>)Vec2<int>(w, h),
		  0
		};
		SubmitForRender(pixel, tr, zLayer);
		currentTint = t;

	}

	void Engine::DrawLine(int startx, int starty, int endx, int endy, Color c, int thickness, float zLayer) {
		Color t = currentTint;
		currentTint = c;

		Vec2<double> delta = {
		  (double)endx - (double)startx,
		  (double)endy - (double)starty
		};

		double ang = atan2(delta.y, delta.x) * (360.0 / PI2);

		delta *= delta;
		double scale = std::sqrt(delta.x + delta.y);

		Transform tr = {
		  (Vec2<float>)Vec2<int>(startx, starty),
		  { 0, (float)round(thickness / 2) + 1.0f },
		  {(float)scale, (float)thickness },
		  ang
		};

		SubmitForRender(pixel, tr, zLayer);

		currentTint = t;
	}

	void Engine::DrawPixel(int x, int y, Color c, float zLayer) {
		Color t = currentTint;
		currentTint = c;

		Transform tr = {
		  (Vec2<float>)Vec2<int>(x, y),
		  { 0.0f },
		  { 1.0f},
		  0.0f
		};

		SubmitForRender(pixel, tr, zLayer);

		currentTint = t;
	}
#pragma endregion

#pragma region RG3GE::Engine::Texture - Functions
	Texture Engine::TextureLoad(const char* filename) {

		Texture ret;
		ret.slot = -1;

		int iSlot = nextFreeTextureSlot();
		if (iSlot == -1) {
			std::cout << "no free textures slots available: " << filename << std::endl;
			return ret;
		};

		TextureSlot* slot = &_texture_slots[iSlot];

		unsigned char* databuffer = stbi_load(filename, &slot->width, &slot->height, &slot->colorchannels, 0);

		if (!databuffer) {
			std::cout << "failed to load texture: " << filename << std::endl;
			return ret;
		}

		GLCALL(glGenTextures(1, &slot->_gl_texture_id));
		GLCALL(glBindTexture(GL_TEXTURE_2D, slot->_gl_texture_id));

		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
		GLCALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));

		GLCALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, slot->width, slot->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, databuffer));
		GLCALL(glGenerateMipmap(GL_TEXTURE_2D));

		stbi_image_free(databuffer);
		slot->users++;

		ret.crop[0] = { 0,                   0, 0, 0 };
		ret.crop[1] = { (float)slot->width,                   0, 1, 0 };
		ret.crop[2] = { (float)slot->width, (float)slot->height, 1, 1 };
		ret.crop[3] = { 0, (float)slot->height, 0, 1 };

		ret.slot = iSlot;

		ret.texture_plane = CreateShape2D(RG3GE::PolyShapes::QUADS, 4, ret.crop);

		return ret;
	}

	void Engine::TextureChangeCrop(Texture& t, int x, int y, int w, int h) {
		if (t.slot == -1) {
			Debug("Warning!!! : texture has no slot assigned");
			return;
		}

		TextureSlot* slot = &_texture_slots[t.slot];
		if (slot->colorchannels <= 0) {
			Debug("slot does not contain a texture");
			return;
		}

		float cw = (float)w / slot->width;
		float ch = (float)h / slot->height;
		float cx = (float)x / slot->width;
		float cy = (float)y / slot->height;

		t.crop[0] = { 0,        0, cx    , cy };
		t.crop[1] = { (float)w,        0, cx + cw, cy };
		t.crop[2] = { (float)w, (float)h, cx + cw, cy + ch };
		t.crop[3] = { 0, (float)h, cx    , cy + ch };

		GLCALL(glBindBuffer(GL_ARRAY_BUFFER, t.texture_plane.vertexBuffer));
		GLCALL(glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex2D), t.crop, GL_DYNAMIC_DRAW));
	}

	void Engine::TextureDraw(Texture& t, Transform& tr, float zLayer) {
		if (t.slot == -1) {
			Debug("Warning!!! : texture has no slot assigned");
			return;
		}

		GLCALL(glUniform1f(_gl_texture_enable_uniform, 1.0f));

		GLCALL(glActiveTexture(GL_TEXTURE0));
		GLCALL(glBindTexture(GL_TEXTURE_2D, _texture_slots[t.slot]._gl_texture_id));
		GLCALL(glUniform1i(_gl_texture_uniform, 0));

		DrawShape2D(t.texture_plane, tr, zLayer);
		GLCALL(glUniform1f(_gl_texture_enable_uniform, 0.0f));
	}

	Texture Engine::TextureClone(Texture& src) {
		Texture ret;

		ret.slot = src.slot;
		ret.crop[0] = src.crop[0];
		ret.crop[1] = src.crop[1];
		ret.crop[2] = src.crop[2];
		ret.crop[3] = src.crop[3];

		ret.texture_plane = CreateShape2D(RG3GE::PolyShapes::QUADS, 4, ret.crop);

		return ret;
	}

	void Engine::TextureDestroy(Texture& t) {
		DestroyShape2D(t.texture_plane);
		if (t.slot != -1) freeTextureSlot(t.slot);
	}
#pragma endregion
}
