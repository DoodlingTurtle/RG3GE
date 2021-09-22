#include <GL/glew.h>

#include "../Engine.h"
#include "../Global.h"
#include "../../engine_config.h"
#include "../Macros.h"

#include "../Transform.h"
#include <iostream>
#include <algorithm>

#include "../vendor/stb_image.h"
#include "./Shader.h"

namespace RG3GE {

//=============================================================================
// TextureSlots
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region TextureSlots
	struct TextureSlot {
		int width = 0, height = 0, colorchannels = 0;
		unsigned int _gl_texture_id = -1;
        Shape2D texture_plane;
		unsigned int users = 0;
	};
	static TextureSlot  _texture_slots[ENGINE_TEXTURE_LIMIT];
	void Engine::freeTextureSlot(unsigned int slot, bool ignoreUsers) {
		if (_texture_slots[slot].users > 0) _texture_slots[slot].users--;

		if (ignoreUsers || _texture_slots[slot].users == 0) {
			GLCALL(glDeleteTextures(1, &(_texture_slots[slot]._gl_texture_id)));
			_texture_slots[slot].width = 0;
			_texture_slots[slot].height = 0;
			_texture_slots[slot].colorchannels = 0;
            DestroyShape2D(_texture_slots[slot].texture_plane); 
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



//=============================================================================
// Renderer
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region Renderer
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

		Color cs = Color(0.0f, 0.0f, 0.0f, 0.0f);
		for (auto j : _render_jobs) {
            if (cs != j.tint) {
                cs = j.tint;
                glUniform4f(shader.u_drawcolor, cs.r, cs.g, cs.b, cs.a);
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



//=============================================================================
// RG3GE::Vec2
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region RG3GE::Vec2
	std::ostream& operator << (std::ostream& os, const Vec2<int>& v) { os << " Vec2<int>(" << v.x << ", " << v.y << ")"; return os; }
	std::ostream& operator << (std::ostream& os, const Vec2<float>& v) { os << " Vec2<float>(" << v.x << ", " << v.y << ")"; return os; }
	std::ostream& operator << (std::ostream& os, const Vec2<double>& v) { os << " Vec2<double>(" << v.x << ", " << v.y << ")"; return os; }
	std::ostream& operator << (std::ostream& os, const Vec2<short>& v) { os << " Vec2<short>(" << v.x << ", " << v.y << ")"; return os; }
#pragma endregion



//==============================================================================
// RG3GE::Angle
//------------------------------------------------------------------------------
//==============================================================================
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



//=============================================================================
// RG3GE::Color
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region RG3GE::Color
	Color::Color(int r, int g, int b, int a)
		: r(r / 255.0f), g(g / 255.0f), b(b / 255.0f), a(a / 255.0f)
	{}
	Color::Color(float r, float g, float b, float a)
		: r(r), g(g), b(b), a(a)
	{}
	bool Color::operator != (Color& c) { return (c.r != r) || (c.g != g) || (c.b != b) || (c.a != a); }
#pragma endregion



//=============================================================================
// RG3GE::Vertex2D
//-----------------------------------------------------------------------------
//=============================================================================
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



//=============================================================================
// RG3GE::Shape2D
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region RG3GE::Shape2D
	Shape2D::Shape2D()
		: shape(PolyShapes::POINTS)
		, vertexCnt(0), vertexBuffer(0)
	{}
#pragma endregion



//=============================================================================
// RG3GE::Engine
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region RG3GE::Engine

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

		//Init OpenGLShaders
        #include "../shaders/universal.h"
        e->program = RG3GE::Core::CreateShader(universal_vs, universal_fs);
        glUseProgram(e->program);

#define srch_uni( f ) e->shader.f = glGetUniformLocation(e->program, #f)
        srch_uni( u_shader_mode );
        srch_uni( u_screen      );
        srch_uni( u_translation );
        srch_uni( u_origin      );
        srch_uni( u_zlayer      );
        srch_uni( u_angle       );
        srch_uni( u_scale       );
        srch_uni( u_textureCrop );
        srch_uni( u_drawcolor   );
        srch_uni( u_texture     );
#undef srch_uni

#define srch_attr( f ) e->shader.f = glGetAttribLocation(e->program, #f)
        srch_attr( a_position );
        srch_attr( a_color    );
        srch_attr( a_uvCoords );
#undef srch_attr


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
        if(_instance) {
            for (int a = 0; a < ENGINE_TEXTURE_LIMIT; a++) {
                if (_texture_slots[a].colorchannels > 0) {
                    _instance->freeTextureSlot(a, true);
                }
            }

			delete _instance;
        }

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
	{}

	Engine::~Engine() {

		DestroyShape2D(pixel);


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

				RenderAll();
				SetTint(WHITE);
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

        void Engine::resizeWindow(int newWidth, int newHeight) {
            if(newWidth > 0 && newHeight > 0) {
                SDL_SetWindowSize(window, newWidth, newHeight);
            }
        }

	void Engine::_applyScreenSize() {

        GLCALL(glUniform2f(shader.u_screen, (float)windowSize.x, (float)windowSize.y));

		Vec2<float> scale = (Vec2<float>)windowSize / (Vec2<float>)origWindowSize;
		if (scale.x > scale.y) scale.x= scale.y;
		else                  scale.y = scale.x;
		windowScale = scale;

		Vec2<float> offset = origWindowSize * scale;
		offset = windowSize - offset;
		windowOffset = offset;

		GLCALL(glViewport(0, 0, (int)windowSize.x, (int)windowSize.y));
	}

	void Engine::_applyTransform(Transform& transform, float zLayer) {
		Vec2<float> rot = (Vec2<float>)transform.rotation.direction;
        Vec2<float> loc = transform.position * 2.0f * windowScale + windowOffset;

        glUniform2f(shader.u_translation, loc.x, loc.y);
		glUniform2f(shader.u_angle, rot.x, rot.y);

		glUniform2f(shader.u_origin, transform.origin.x, transform.origin.y);
		glUniform1f(shader.u_zlayer, zLayer);
		glUniform2f(shader.u_scale, transform.scale.x * 2 * windowScale.x, transform.scale.y * 2 * windowScale.y);
	}
#pragma endregion



//=============================================================================
// RG3GE::Engine::Input - Functions
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region RG3GE::Engine::Input - Functions

	bool Engine::keyPressed(SDL_Keycode code) { return keys_pressed.find(code) != keys_pressed.end(); }
	bool Engine::keyReleased(SDL_Keycode code) { return keys_released.find(code) != keys_released.end(); }
	bool Engine::keyHeld(SDL_Keycode code) { return keys_held.find(code) != keys_held.end(); }

	bool Engine::mousePressed(Uint8 code) { return mouse_pressed.find(code)  != mouse_pressed.end();  }
	bool Engine::mouseHeld(Uint8 code)    { return mouse_held.find(code)     != mouse_held.end();  }
	bool Engine::mouseReleased(Uint8 code){ return mouse_released.find(code) != mouse_released.end();  }

#pragma endregion



//=============================================================================
// RG3GE::Engine::Shape2D - Functions
//-----------------------------------------------------------------------------
//=============================================================================
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
        
        glUniform1i(shader.u_shader_mode, 0);

		glEnableVertexAttribArray(shader.a_position);
		glEnableVertexAttribArray(shader.a_color);
		glEnableVertexAttribArray(shader.a_uvCoords);
		glEnableClientState(GL_VERTEX_ARRAY);
		glPushMatrix();

		_applyTransform(tr, zLayer);

		glBindBuffer(GL_ARRAY_BUFFER, shape.vertexBuffer);
		glVertexPointer(2, GL_FLOAT, 0, NULL);
		glVertexAttribPointer(shader.a_position, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), 0);
		glVertexAttribPointer(shader.a_color,    4, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), (void*)(2 * sizeof(GL_FLOAT)));
		glVertexAttribPointer(shader.a_uvCoords, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), (void*)(6 * sizeof(GL_FLOAT)));
		glDrawArrays(static_cast<GLint>(shape.shape), 0, shape.vertexCnt);

		glPopMatrix();
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableVertexAttribArray(shader.a_position);
		glDisableVertexAttribArray(shader.a_color);
        glDisableVertexAttribArray(shader.a_uvCoords);
	}
#pragma endregion



//=============================================================================
// RG3GE::Engine::Draw... - Functions
//-----------------------------------------------------------------------------
//=============================================================================
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


	void Engine::DrawLine( int startx, int starty, int endx, int endy, Color c, int thickness, float zLayer) {
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



//=============================================================================
// RG3GE::Engine::Texture - Functions
//-----------------------------------------------------------------------------
//=============================================================================
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
        slot->texture_plane = CreateShape2D(RG3GE::PolyShapes::QUADS, {
            {              0.0f,                0.0f, 0.0f, 0.0f},
            {(float)slot->width,                1.0f, 1.0f, 0.0f},
            {(float)slot->width, (float)slot->height, 1.0f, 1.0f},
            {              0.0f, (float)slot->height, 0.0f, 1.0f}
        });
		slot->users++;

        ret.cropSize.x = 1.0f;
        ret.cropSize.y = 1.0f;
        ret.uvOffset.x = 0;
        ret.uvOffset.y = 0;
		ret.slot = iSlot;

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

        t.cropSize.x = (float)w / slot->width;
        t.cropSize.y = (float)h / slot->height; 
        t.uvOffset.x = (float)x / slot->width;
        t.uvOffset.y = (float)y / slot->height;
	}

	void Engine::TextureDraw(Texture& t, Transform& tr, float zLayer) {
		if (t.slot == -1) {
			Debug("Warning!!! : texture has no slot assigned");
			return;
		}

        glUniform1i(shader.u_shader_mode, 1);

		glEnableVertexAttribArray(shader.a_position);
        glEnableVertexAttribArray(shader.a_color);
		glEnableVertexAttribArray(shader.a_uvCoords);
		glEnableClientState(GL_VERTEX_ARRAY);

		glPushMatrix();

		_applyTransform(tr, zLayer);
        glUniform4f(
            shader.u_textureCrop, 
            t.cropSize.x,
            t.cropSize.y,
            t.uvOffset.x,
            t.uvOffset.y 
        );

		glBindBuffer(GL_ARRAY_BUFFER, _texture_slots[t.slot].texture_plane.vertexBuffer);
		glVertexPointer(2, GL_FLOAT, 0, NULL);
		glVertexAttribPointer(shader.a_position, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), 0);
		glVertexAttribPointer(shader.a_color,    4, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), (void*)(2 * sizeof(GL_FLOAT)));
		glVertexAttribPointer(shader.a_uvCoords, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex2D), (void*)(6 * sizeof(GL_FLOAT)));

		glActiveTexture(GL_TEXTURE0);

		glBindTexture(GL_TEXTURE_2D, _texture_slots[t.slot]._gl_texture_id);
		glUniform1i(shader.u_texture, 0);
		glDrawArrays(static_cast<GLint>(_texture_slots[t.slot].texture_plane.shape), 0, _texture_slots[t.slot].texture_plane.vertexCnt);

		glBindTexture(GL_TEXTURE_2D, 0);

		glPopMatrix();

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableVertexAttribArray(shader.a_position);
        glDisableVertexAttribArray(shader.a_color);
		glDisableVertexAttribArray(shader.a_uvCoords);

	}


	Texture Engine::TextureClone(Texture& src) {
		Texture ret;

		ret.slot = src.slot;
        ret.cropSize = src.cropSize;
        ret.uvOffset = src.uvOffset;

		return ret;
	}

	void Engine::TextureDestroy(Texture& t) {
		if (t.slot != -1) freeTextureSlot(t.slot);
	}
#pragma endregion


//=============================================================================
// Global-Setup
//-----------------------------------------------------------------------------
//=============================================================================
#pragma region Global-Setup
namespace Global {

#define __GLOBALS_HEADERS__
#include "../../engine_globals.hpp"
#undef __GLOBALS_HEADERS__

#define REGISTER_GLOBAL(t, n, v) t n = v;
#include "../../engine_globals.hpp"
#include "./global.hpp"
}
#pragma endregion

}


