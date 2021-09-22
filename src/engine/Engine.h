#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <functional>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <SDL2/SDL_opengl.h>

namespace RG3GE {

	struct Transform;

	/**
	 * Defines how Shape2D Objects are draw.
	 */
	enum class PolyShapes {
		POINTS = GL_POINTS,
		LINES = GL_LINES,
		LINE_LOOP = GL_LINE_LOOP,
		LINE_STRIP = GL_LINE_STRIP,
		TRIANGLES = GL_TRIANGLES,
		TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
		TRIANGLE_FAN = GL_TRIANGLE_FAN,
		QUADS = GL_QUADS,
		QUAD_STRIP = GL_QUAD_STRIP,
		POLYGON = GL_POLYGON
	};

	/**
	 * Vector based graphic constructs , that are stored in VRAM.
	 */
	struct Shape2D {
		RG3GE::PolyShapes shape;
		int vertexCnt;
		unsigned int vertexBuffer;

		Shape2D();
	};

	/**
	 * Your Typical 2 Value Vector.
	 */
	template <typename T>
	struct Vec2 {
		T x, y;

		Vec2<T>  operator + (Vec2<T> o) { return Vec2<T>{ this->x + o.x, this->y + o.y}; }
		Vec2<T>  operator - (Vec2<T> o) { return Vec2<T>{ this->x - o.x, this->y - o.y}; }
		Vec2<T>  operator * (Vec2<T> o) { return Vec2<T>{ this->x* o.x, this->y* o.y}; }
		Vec2<T>  operator / (Vec2<T> o) { return Vec2<T>{ this->x / o.x, this->y / o.y}; }

		Vec2<T>& operator += (Vec2<T> o) { this->x += o.x; this->y += o.y; return *this; }
		Vec2<T>& operator -= (Vec2<T> o) { this->x -= o.x; this->y -= o.y; return *this; }
		Vec2<T>& operator *= (Vec2<T> o) { this->x *= o.x; this->y *= o.y; return *this; }
		Vec2<T>& operator /= (Vec2<T> o) { this->x /= o.x; this->y /= o.y; return *this; }

		operator Vec2<float>() { return Vec2<float>{ static_cast<float>(x), static_cast<float>(y) }; }
		operator Vec2<double>() { return Vec2<double>{ static_cast<double>(x), static_cast<double>(y) }; }
		operator Vec2<int>() { return Vec2<int>{ static_cast<int>(x), static_cast<int>(y) }; }

		Vec2<T>(T x, T y) { this->x = x; this->y = y; };
		Vec2<T>(T v = 0) { this->x = v; this->y = v; };

		void fillArray(T arr[2]) {
			arr[0] = this->x;
			arr[1] = this->y;
		}
	};


	/**
	 * This should not need an explaination
	 */
	struct Color {
		float r, g, b, a;
		Color(float r, float g, float b, float a);
		Color(int r, int g, int b, int a);
		bool operator != (Color& c);
	};

	/**
	 * Tells the GPU how and where to paint a vertex.
	 */
	struct Vertex2D {
		Vec2<float> position;
		Color vertexColor;
		Vec2<float> uvCoords;

		Vertex2D();
		Vertex2D(float x, float y);
		Vertex2D(float x, float y, int r, int g, int b, int a);
		Vertex2D(float x, float y, Color c);
		Vertex2D(float x, float y, float u, float v);
	};

	struct Texture {
		Vec2<float> cropSize;
        Vec2<float> uvOffset;
		int slot;
	};

    struct Shader {
        int u_shader_mode;

        int u_screen;
        
        int u_translation;
        int u_origin;
        int u_zlayer;
        int u_angle;
        int u_scale;

        int u_textureCrop;

        int u_drawcolor;
        int u_texture;

        int a_position;
        int a_color;
        int a_uvCoords;
    };

	/**
	 * Heartpiece of the the Engine.
	 */
	class Engine {
	public:
		static const Color WHITE;
		static const Color BLACK;

		/** Changes that the letter box border will look like.  */
		Color borderColor;

		/**
		 * \brief Initialize the Module and create the application Window
		 *
		 * \param winWidth
		 * \param winHeight
		 * \param winTitle
		 * \param onBuild - a callback, that can be used to check various definitions or define global values
		 *                  before the game starts
		 *                  (if this returns false, the game will not start and end emidiately)
		 *
		 * \param sdl_init_flags - SDL2 is part of this initialization
		 *						 All Flags, that Engine needs will be set
		 *						 Allready, but you can add additional flags here
		 *
		 *						 Flags set by Engine are:
		 *						 - SDL_INIT_VIDEO
		 *						 - SDL_INIT_TIMER
		 *
		 * \return - the instance, that can then be used to make various Draw-Calls and so on
		 */
		static Engine* init(
			int winWidth, int winHeight,
			const char* winTitle = "Application",
			unsigned int sdl_init_flags = 0,
			std::function<bool(const Engine*)> onBuild = [](const Engine* e) {return true; }
		);

		/**
		 * \brief Destorys all still running instances of engine and shuts down
		 *		SDL2
		 */
		static void cleanup();

		/**
		 * \return  - the time passed, between the last 2 windowTick() calls
		 */
		float deltaTime();

		/**
		 * Updates Game and the deltaTime()
		 *
		 * \return - true = game is still running
		 *           false = the game has ended. Further windowTick() calls are useless
		 */
		bool windowTick();

		/**
		 * Clears the screen to the given color also reset OpenGL Output
		 *
		 * \param c
		 */
		void ClearScreen(Color c = { 0.0f, 0.0f, 0.0f, 1.0f });

		/**
		 * Multiplys the given output Color to what ever is rendered on the screen
		 * \param c
		 */
		void SetTint(Color c);

		/*==============================================================================
		 * 2D Polygon related functions
		 *============================================================================*/
		/** \brief Creates a new Shape Object (store in VRAM), based on a buch of vertex corrdinates
		 * \param PolyShape tells the GPU how to interpret the points given
		 * \param verts - the total number of points, that are stored in the GPU
		 * \param vertices - vertices sent to the GPU
		 */
		Shape2D CreateShape2D(PolyShapes shape, int verts, const Vertex2D vertices[]);
		Shape2D CreateShape2D(PolyShapes shape, const std::vector<Vertex2D>& data);

		/** \Shapes created via CreateShape2D need to be destory, (to free the Graphics Card)
		 */
		void DestroyShape2D(Shape2D);

		// Basic Draw Functions
		void DrawPixel(int x, int y, Color c, float zLayer = 0);
		void DrawRectFilled(int x, int y, int w, int h, Color c, float zLayer = 0);
		void DrawLine(int startx, int starty, int endx, int endy, Color c, int tickness = 1, float zLayer = 0);

		/*==============================================================================
		 * Texture related functions
		 *============================================================================*/
		/**
		 * Loads a Texture from a File (supported tested file type(s) is/are .png .
		 * 
		 * \param filename - filename relative to the .executeable
		 * 
		 * \return - the Resource-Handle for the texture
		 */
		Texture TextureLoad(const char* filename);

		/** Loaded textures need to be destroyed, (to free Up VRAM) */
		void    TextureDestroy(Texture& t);
		
		void SubmitForRender(Texture&, Transform&, float zLayer = 0);
		void SubmitForRender(Shape2D&, Transform&, float zLayer = 0);

		void RenderAll();

		void	TextureChangeCrop(Texture& t, int x, int y, int w, int h);

		Texture TextureClone(Texture& src);


        /*==============================================================================
         * Window Functions
         *============================================================================*/
        void resizeWindow(int newWidth, int newHeight);

		/*==============================================================================
		 * Keyboard functions
		 *============================================================================*/
		bool keyPressed(SDL_Keycode code);
		bool keyHeld(SDL_Keycode code);
		bool keyReleased(SDL_Keycode code);

		bool mousePressed (Uint8 code);
		bool mouseHeld    (Uint8 code);
		bool mouseReleased(Uint8 code);

		virtual ~Engine();

		Vec2<float> mousePosition;
    
	private:

		/** \brief what this does should be self explainatory
		 * \param Shape2D - the shape to draw
		 * \param Transform - the transformation will be taken into account
		 * \param zLayer - value from -1 to 0.999999
		 *                 will be drawn over all elements, that have a higher number then this
		 */
		void DrawShape2D(Shape2D, Transform& tr, float zLayer = 0);

		/** Outputs the texture to the screen, on the given zLayer under use of the given transformation.  */
		void	TextureDraw(Texture& t, Transform&, float zLayer = 0);


        void freeTextureSlot(unsigned int slot, bool ignoreUsers = false);

		Engine();

		static Engine* _instance;

		Color currentTint;

		SDL_GLContext context;
		SDL_Window* window;

		bool keepRunning;
		SDL_Event event;

		float _deltaTime;
		Vec2<float> windowSize;
		Vec2<float> origWindowSize;
		Vec2<float> windowOffset;
		Vec2<float> windowScale;
		Uint32 ticks;

		void _applyTransform(Transform& tr, float zLayer);
		void _applyScreenSize();

		Shape2D pixel;
		Shape2D line;

		std::unordered_set<SDL_Keycode> keys_pressed;
		std::unordered_set<SDL_Keycode> keys_released;
		std::unordered_set<SDL_Keycode> keys_held;
		SDL_Keycode last_pressed;

		std::unordered_set<Uint8> mouse_pressed;
		std::unordered_set<Uint8> mouse_released;
		std::unordered_set<Uint8> mouse_held;

        int program;
        Shader shader;
	};

}
