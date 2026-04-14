#include "core/Application.h"
#include "scenes/MainMenuScene.h"

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
			AudioSource
		>();

		GetSceneManager().Push<MainMenuScene>(GetRegistry(), GetAssets());
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