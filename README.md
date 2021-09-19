# RG3GE 
A project, that started as a leaning experience for OpenGL.
Somehow it grew into a little GameEngine like thing.
(Since I use SDL2 to provide the window and drawing context, why
not use the rest of it too. 😉)

At the moment it is more of a glorified OpenGL wrapper, that provides 
the bare minimum needed to make a application. (see the `src/main.cpp`) 

To see what features are availabl check the `src/engine/Engine.h`.

### Available functions
- Loading textures
- Drawing polygon based 2D shapes
- Processing inputs for keyboard and mouse 
- Providing a `deltaTime` modifier for Framerate independed processing
- Provides a "Transform" - component, that allows for easy manipulation of rotations, scales and locations.

### How to use it:
- put the `src/engine` folder into your project
- put the `src/engine_config.h` next to the `engine` - folder
  - you can make changes to the configuration in the file (if needed)

- in your `main` function, you can use the engine as follows:
```cpp
                        //  WinWidth, WinHeight, WinTitle, SDL_Flags
Engine* game = Engine::init(256,      256,      "Demo",    0 /*or additional SDL_Init - Flags, if you need them*/);

if (game) {
    // OnStart Area
    // .. initialize your ressource here

    while (game->windowTick()) {
        float deltaTime = game->deltaTime(); // time passed since the last windowTick()

        // ...
        // do all your Main-Game stuff here ( updates, rendering, etc. ).
        // See src/main.cpp for an example
        //
        // you can look at 'src/engine/Engine.h' to see, what you can do.
        // All other SDL2 functions are awailable as well

     }

    // On End Area
    // .. Destroy your ressource here
}

```

### How to compile it:
- Make sure your compiler has access to SDL2 (`-lSDL2 -lSDL2main`) \
 ,OpenGL and GLEW (`-lglew32 -lopengl32`)
- Put the `SDL2.dll` and `glew32.dll` into the same folder, where your compiled binary will be.

- If you use linux or msys, then you can use the Makefile in this project\
to compile it.
  - use `make` to create a debug build.
  - use `make run` to create and start the debug build.
  - use `make release` to create a build, that does not open a terminal in the background and requires less ressources to run.
  - use `make clean` to remove all compiled files

### TODO:
- ⬜ Draw distoted textures (by manipulating the texture Plane/mesh) 
- ⬜ Implement the Scene-System (from [WASM_WASteroids](https://github.com/DoodlingTurtle/WASM_WAsteroids))
- ⬜ Implement the Global-System (from [WASM_WASteroids](https://github.com/DoodlingTurtle/WASM_WAsteroids))
- ...
- ⬜ Documentation 

## DONE:
- ✔ Figure out, how to compile without VS2019
- ✔ Create SDL-Window and Close-Event
- ✔ Create OpenGL-Context to draw in
- ✔ Reset the screen in various colors
- ✔ Draw basic 2d shapes (via Polys)
- ✔ Draw shapes more efftiently (by storing in them in GPU, instead of sending each vertex on every cycle)
- ✔ build own shaders
- ✔ Make window resizeable
- ✔ Learn to draw 2d textures (pictures)
- ✔ Keyboard input
- ✔ MouseInput
- ✔ combine Textures with Transforms
- ✔ Learn to draw partial textures
- ✔ Pack everything into a nice API to make it a part of the engine
