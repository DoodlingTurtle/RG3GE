#include "./engine/Engine.h"
#include "./engine/Transform.h"
#include "./engine/Macros.h"

using namespace RG3GE;


int main(int argc, char** argv) {

// Colors
	Color darkcyan  = Color(0, 128, 128, 255);
	Color red		= Color(255, 0, 0, 255);
	Color darkred	= Color( 128, 0, 0, 255 );
	Color orange    = Color( 160, 96, 32, 255 );
	Color green     = Color(0, 255, 0, 32);


// Setup Window
	Engine* game = Engine::init(256, 256, "RG3GE::Engine DEMO");

	if (game) {
		// OnStart Area

		// Load Textures into VRAM
		Texture ships = game->TextureLoad("./assets/ship.png");
		Transform tr_spritesheet = { { 128 }, { 32, 48 }, {1.0f}, 0.0f };

		// Create Sprites from Texture
		Texture spr_player_ship = game->TextureClone(ships);
		game->TextureChangeCrop(spr_player_ship, 0, 0, 32, 32);
		Transform tr_player_ship = { {28}, {16}, {1.0f}, 45.0f };

		Texture spr_enemy_ship = game->TextureClone(ships);
		game->TextureChangeCrop(spr_enemy_ship, 0, 64, 32, 32);
		Transform tr_enemy_ship = { {256-28}, {16}, {1.0f}, -45.0f };

		// Create Polygon Object
		Shape2D ship = game->CreateShape2D(PolyShapes::TRIANGLE_STRIP, {
			{  0, 32, darkred},
			{ 16, 24, red },
			{ 16,  0, red },
			{ 32, 32, darkred}
		});
		Transform tr_shape = { { 256 - 64, 64 }, { 16 }, { 2.0f } , -135.0f };

		std::vector<Transform*> rotators = { &tr_player_ship, &tr_enemy_ship, &tr_shape };
		std::vector<Transform*> scalers = { &tr_shape, &tr_enemy_ship };
		std::vector<Transform*> halfscalers = { &tr_spritesheet, &tr_player_ship };

		// This is your Main-Loop (game->tick will advance the game by exactly one cycle)
		// if it returns false, the engine has stopped running

		game->borderColor = orange;
		float timePass = 0.0f;

		while (game->windowTick()) {

			float deltaTime = game->deltaTime();
			timePass += deltaTime;

			for (auto t : rotators)    t->rotation += 36.0 * deltaTime; // One turn every 10 Seconds
			for (auto t : scalers)     t->scale = sin(timePass) + 2.0f;
			for (auto t : halfscalers) t->scale = { 1.0f, sin(timePass) + 2.0f } ;

			game->ClearScreen(darkcyan);

			game->SubmitForRender(spr_player_ship, tr_player_ship);
			game->SubmitForRender(spr_enemy_ship,  tr_enemy_ship);
			game->SubmitForRender(ships, tr_spritesheet, 0.0001f);
			game->SubmitForRender(ship, tr_shape);

			game->DrawLine(0, 0, 256, 256, green, 1, 0.9999f);
			game->DrawLine(256, 0, 0, 256, green, 1, 0.9999f);
			game->DrawRectFilled(112, 112, 32, 32, orange, 0.9999f);

		}

		// OnEnd Area
		game->TextureDestroy(spr_enemy_ship);
		game->TextureDestroy(spr_player_ship);
		game->TextureDestroy(ships);
		game->DestroyShape2D(ship);
	}

	Engine::cleanup();

	return 0;
}
