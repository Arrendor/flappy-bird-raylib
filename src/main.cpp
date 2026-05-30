#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "raymath.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir


class Player 
{
private:
	static Vector2 pos;
	static Vector2 force;
	static bool alive;
	static float jumpHeight;
	static float gravityStrength;
public:
	Player()
	{
	}
	static Vector2 getPos()
	{
		return pos;
	}
	static void Move(Vector2 target) 
	{
		pos = target;
	}
	static void ResetForce()
	{
		force = Vector2Zero();
	}
	static void Jump()
	{
		force.y = -(jumpHeight);
	}
	static void DoPhysics()
	{
		//Gravity
		Move(pos+force);
		force.y += gravityStrength * GetFrameTime();
	}
};

class PipePair
{
private:
	static float pipeSpeed;
	static float pipeGap;
	float xPos;
	float yTop;
	float yBot;
	float yCenter;
	bool passed;
public:
	PipePair()
	{
		xPos = GetScreenWidth() + 200;
		yCenter = GetRandomValue(GetScreenHeight() / 2.0f - 100, GetScreenHeight() / 2.0f + 100);
		yBot = yCenter + pipeGap / 2;
		yTop = yCenter - pipeGap / 2;
		passed = false;
	}
	float getX() const
	{
		return xPos;
	}
	float getBot() const
	{
		return yBot;
	}
	float getTop() const
	{
		return yTop;
	}
	float getCenter() const
	{
		return yCenter;
	}
	bool hasPassed() const
	{
		return passed;
	}
	void SetPassed(bool value)
	{
		passed = value;
	}
	void Scroll()
	{
		xPos -= pipeSpeed;
	}
	void DoPhysics()
	{
		Scroll();
	}
};
class Timer
{
private:
	float length;
	float progress;
	bool repeating;
	bool active;
	bool finished;
	
public:
	Timer(float length, bool repeating)
	{
		this->length = length;
		this->repeating = repeating;
		progress = 0;
		active = true;
		finished = false;
	}
	void Update()
	{
		if (active)
		{
			finished = false;
			progress += GetFrameTime();
			if (progress > length && repeating)
			{
				progress = 0;
				finished = true; //will be true for one frame
			}
			else if (progress > length)
			{
				active = false;
				finished = true;
			}
		}
	}
	void SetProgress(float progress)
	{
		this->progress = progress;
	}
	bool is_finished() const
	{
		return finished;
	}
};

Vector2 Player::pos = Vector2Zero();
Vector2 Player::force = Vector2Zero();
bool Player::alive = true;
float Player::jumpHeight = 7.5;
float Player::gravityStrength = 19;

float PipePair::pipeSpeed = 3;
float PipePair::pipeGap = 700;

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(800, 600, "Flappy Bird with RayLib");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");
	wabbit.height = wabbit.height * 2;
	wabbit.width = wabbit.width * 2;
	Rectangle wabbit_rect = { 0, 0, wabbit.width, wabbit.height };
	Rectangle wabbit_hitrect = { 0, 0, wabbit.width - 20, wabbit.height - 10 };
	Rectangle wabbit_hb = { 0, 0, wabbit.width, wabbit.height };

	Texture pipe_texture = LoadTexture("pipe.png");
	pipe_texture.height = pipe_texture.height / 4;
	pipe_texture.width = pipe_texture.width / 4;
	Rectangle pipe_rect = { 0, 0, pipe_texture.width, pipe_texture.height };
	Rectangle pipe_rect_c = { -pipe_texture.width / 2, -pipe_texture.height / 2,
		pipe_texture.width, pipe_texture.height};
	
	Texture debugt = LoadTexture("black.png");
	//Timers
	Timer frameTimer = Timer(0.25f, true);
	std::string frameText = "";
	float frameSum = 0;
	int frameCountSinceDisplay = 0;

	Timer pipeTimer = Timer(1.5f, true);
	pipeTimer.SetProgress(1.6f);

	//Setup
	std::vector<PipePair> allPipes;
	allPipes.reserve(4);

	bool collision = false;

	Rectangle collisionBox = {};

	int score = 0;
	std::string scoreText = "SCORE: 0";

	Player::Move(Vector2{ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f });

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		//pipes
		pipeTimer.Update();
		if (pipeTimer.is_finished())
		{
			allPipes.emplace_back(PipePair());
		}

		//input
		if (IsKeyPressed(KEY_SPACE)) Player::Jump();

		//physics
		frameTimer.Update();
		frameSum += GetFrameTime();
		frameCountSinceDisplay++;
		if (frameTimer.is_finished())
		{
			frameText = std::to_string(int(round(frameCountSinceDisplay / frameSum)));
			frameSum = 0;
			frameCountSinceDisplay = 0;
		}

		for (int i = 0; i < allPipes.size(); i++)
		{
			if (!allPipes[i].hasPassed() && allPipes[i].getX() < Player::getPos().x + wabbit_hitrect.width / 2)
			{
				score++;
				scoreText = "SCORE: " + std::to_string(score);
				allPipes[i].SetPassed(true);
			}
			if (allPipes[i].getX() < -200)
			{
				allPipes.erase(allPipes.begin() + i);
			}
			else
			{
				allPipes[i].Scroll();
			}
		}

		Player::DoPhysics();

		collisionBox = {};

		wabbit_hb = { Player::getPos().x - wabbit_hitrect.width / 2, Player::getPos().y - wabbit_hitrect.height / 2,
			wabbit_hitrect.width, wabbit_hitrect.height };

		//Collision checking
		if (Player::getPos().y < -50 || Player::getPos().y > GetScreenHeight() + 50)
		{
			collision = true;
		}

		for (const PipePair& pipepair : allPipes)
		{
			Rectangle bot_hb = { pipepair.getX() - pipe_rect.width / 2.0f, pipepair.getBot() - pipe_rect.height / 2.0f,
				pipe_rect.width, pipe_rect.height };
			Rectangle top_hb = { pipepair.getX() - pipe_rect.width / 2.0f, pipepair.getTop() - pipe_rect.height / 2.0f,
				pipe_rect.width, pipe_rect.height };
			Rectangle bot_hb1 = {};
			if (CheckCollisionRecs(bot_hb, wabbit_hb))
			{
				collision = true;
				collisionBox = GetCollisionRec(bot_hb, wabbit_hb);
				break;
			}
			else if (CheckCollisionRecs(top_hb, wabbit_hb))
			{
				collision = true;
				collisionBox = GetCollisionRec(top_hb, wabbit_hb);
				break;
			}
		}

		if (collision)
		{
			
			Player::ResetForce();
			Player::Move(Vector2{ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f });
			collision = false;
			allPipes.clear();
			score = 0;
			scoreText = "SCORE: " + std::to_string(score);
		}

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(RAYWHITE);

		// draw some text using the default font
		

		// draw our texture to the screen
		DrawTexturePro(wabbit, wabbit_rect, Rectangle{ Player::getPos().x, Player::getPos().y, wabbit_rect.width, wabbit_rect.height },
			Vector2{ wabbit_rect.width / 2, wabbit_rect.height / 2 }, 0, WHITE);

		//DrawTexture(pipe, Player::getPos().x, Player::getPos().y, WHITE);
		for (const PipePair& pipepair : allPipes)
		{
			DrawTexturePro(pipe_texture, pipe_rect, Rectangle{ pipepair.getX(), pipepair.getBot(), pipe_rect.width, pipe_rect.height },
				Vector2{ pipe_rect.width / 2, pipe_rect.height / 2 }, 0, WHITE);
			DrawTexturePro(pipe_texture, pipe_rect, Rectangle{ pipepair.getX(), pipepair.getTop(), pipe_rect.width, pipe_rect.height },
				Vector2{ pipe_rect.width / 2, pipe_rect.height / 2 }, 180, WHITE);
		}
		/*for (PipePair pipepair : allPipes)
		{
			Rectangle bot_hb = { pipepair.getX() - pipe_rect.width / 2.0f, pipepair.getBot() - pipe_rect.height / 2.0f,
				pipe_rect.width, pipe_rect.height };
			Rectangle top_hb = { pipepair.getX() - pipe_rect.width / 2.0f, pipepair.getTop() - pipe_rect.height / 2.0f,
				pipe_rect.width, pipe_rect.height };
			DrawRectangleRec(bot_hb, DARKGREEN);
			DrawRectangleRec(top_hb, RED);
		}
		
		DrawRectangleRec(collisionBox, BLUE);*/

		//Overlaid
		DrawText(frameText.c_str(), 5, 0, 16, BLACK);
		DrawText(scoreText.c_str(), 5, 20, 30, BLACK);
		// end the frame and get ready for the next one  (display frame, poll input, etc...)

		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);
	UnloadTexture(pipe_texture);
	UnloadTexture(debugt);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
