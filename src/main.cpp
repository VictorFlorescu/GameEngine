#include "core/Application.h"
#include "scenes/MainMenuScene.h"
#include "scenes/GameplayScene.h"

class Game : public Application
{
public:
	Game() : Application({"Game", 1280, 720}) {}

protected:
	void OnStart() override
	{
		GetRegistry().RegisterComponents<
			Spatial,
			Sprite,
			MeshRenderer,
			Rigidbody,
			AudioSource,
			BoxCollider,
			Camera2DComponent
		>();

		GetSceneManager().Push<GameplayScene>(GetRegistry(), GetAssets());
	}

	void OnUpdate(float deltaTime) override
	{
		if (GetInput().IsKeyPressed(KEY_F11))
			ToggleFullscreen();
	}
};

int main()
{
	Game game;
	game.Run();
	return 0;
}