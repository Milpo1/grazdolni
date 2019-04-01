#include <SDL2/SDL.h>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <cmath>

#define TILE_SIZE 32

// check for errors
#define errcheck(e)                   \
  {                                   \
    if (e)                            \
    {                                 \
      cout << SDL_GetError() << endl; \
      SDL_Quit();                     \
      return -1;                      \
    }                                 \
  }

struct game_map_t
{
	int w, h;
	std::vector<std::string> tiles;
	int start_x, start_y;
};

enum player_dir_e
{
	LEFT,
	RIGHT,
	UP,
	DOWN
};
struct player_t
{
	double x, y, hv, vv, movespd = 0.5;
	player_dir_e dir;
};

game_map_t load_map(const std::string map_name)
{
	using namespace std;
	fstream f(map_name, ios_base::in);
	game_map_t ret_map;
	f >> ret_map.w >> ret_map.h;
	for (unsigned int i = 0; i < ret_map.h; i++)
	{
		string line;
		f >> line;
		ret_map.tiles.push_back(line);
		for (int x = 0; x < ret_map.w; x++)
		{
			if (line.at(x) == '@')
			{
				ret_map.start_x = x;
				ret_map.start_y = i;
			}
		}
	}
	return ret_map;
}

//bool pnt_in_rect()

void debug_print_map(game_map_t m)
{
	using namespace std;
	cout << m.w << " " << m.h << "   start: " << m.start_x << ", " << m.start_y
		<< endl;
	for (auto l : m.tiles)
	{
		cout << l << endl;
	}
}

void draw_map(SDL_Renderer *renderer, SDL_Texture *tex, const game_map_t &m)
{

	for (unsigned y = 0; y < m.h; y++)
	{
		for (unsigned x = 0; x < m.w; x++)
		{
			SDL_Rect rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };

			switch (m.tiles.at(y).at(x))
			{
			case '@':
			case '.':
				SDL_RenderCopy(renderer, tex, NULL, &rect);
				//SDL_SetRenderDrawColor(renderer, 100, 60, 20, 255);
				break;
			case '#':
				SDL_RenderCopy(renderer, tex, NULL, &rect);
				//SDL_SetRenderDrawColor(renderer, 40, 40, 255, 255);
				break;
			case '*':
				SDL_RenderCopy(renderer, tex, NULL, &rect);
				//SDL_SetRenderDrawColor(renderer, 40, 40, 70, 255);
				break;
			case 'x':
				SDL_RenderCopy(renderer, tex, NULL, &rect);
				//SDL_SetRenderDrawColor(renderer, 255, 128, 128, 255);
				break;
			}
			/*if (m.tiles.at(y).at(x) != '*')
			{
			SDL_RenderFillRect(renderer, &rect);
			SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
			SDL_Rect rectf = {(int)(x * TILE_SIZE), (int)(y * TILE_SIZE), TILE_SIZE, TILE_SIZE};
			SDL_RenderDrawRect(renderer, &rectf);
			}*/
		}
	}
}

void draw_player(SDL_Renderer *renderer, SDL_Texture *player_texture, const player_t &p)
{
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect rect = { (int)(p.x * TILE_SIZE + 1),
		(int)(p.y * TILE_SIZE + 1),
		TILE_SIZE - 2,
		TILE_SIZE - 2 };
	/*SDL_RenderFillRect(renderer, &rect);*/
	SDL_RenderCopy(renderer, player_texture, NULL, &rect);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	switch (p.dir)
	{
	case LEFT:
		SDL_RenderDrawLine(renderer, (p.x + 0.5) * TILE_SIZE,
			(p.y + 0.25) * TILE_SIZE, (p.x + 0.5 - 2) * TILE_SIZE,
			(p.y + 0.25) * TILE_SIZE);
		break;
	case RIGHT:
		SDL_RenderDrawLine(renderer, (p.x + 0.5) * TILE_SIZE,
			(p.y + 0.25) * TILE_SIZE, (p.x + 0.5 + 2) * TILE_SIZE,
			(p.y + 0.25) * TILE_SIZE);
		break;
	case UP:
		SDL_RenderDrawLine(renderer, (p.x + 0.5) * TILE_SIZE,
			(p.y + 0.25) * TILE_SIZE, (p.x + 0.5) * TILE_SIZE,
			(p.y + 0.25 - 2) * TILE_SIZE);
		break;
	case DOWN:
		SDL_RenderDrawLine(renderer, (p.x + 0.5) * TILE_SIZE,
			(p.y + 0.25) * TILE_SIZE, (p.x + 0.5) * TILE_SIZE,
			(p.y + 0.25 + 2) * TILE_SIZE);
		break;
	}
}

int main(int, char **)
{
	using namespace std;
	using namespace std::chrono;
	const int width = 640;
	const int height = 480;

	errcheck(SDL_Init(SDL_INIT_VIDEO) != 0);

	SDL_Window *window = SDL_CreateWindow(
		"My Next Superawesome Game", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	errcheck(window == nullptr);

	SDL_Renderer *renderer = SDL_CreateRenderer(
		window, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_PRESENTVSYNC
	errcheck(renderer == nullptr);
	SDL_Surface *surface = SDL_LoadBMP("data/player.bmp");
	SDL_Texture* player_texture = SDL_CreateTextureFromSurface(renderer, surface);
	surface = SDL_LoadBMP("data/papryka.bmp");
	SDL_Texture* papryka_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = NULL;

	// auto dt = 15ms;
	milliseconds dt(15);

	auto game_map = load_map("data/level0");
	player_t player;
	player.x = game_map.start_x;
	player.y = game_map.start_y;
	debug_print_map(game_map);

	steady_clock::time_point current_time =
		steady_clock::now(); // remember current time
	for (bool game_active = true; game_active;)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{ // check if there are some events
			if (event.type == SDL_QUIT)
				game_active = false;
		}

		const Uint8 *state = SDL_GetKeyboardState(NULL);
		auto new_player_pos = player;
		if (state[SDL_SCANCODE_LEFT])
		{
			new_player_pos.hv = -0.1;
			new_player_pos.dir = LEFT;
		}
		if (state[SDL_SCANCODE_RIGHT])
		{
			new_player_pos.hv = TILE_SIZE;
			new_player_pos.dir = RIGHT;
		}
		if (state[SDL_SCANCODE_UP])
		{
			new_player_pos.vv = -0.1;
			new_player_pos.dir = UP;
		}
		if (state[SDL_SCANCODE_DOWN])
		{
			new_player_pos.vv = TILE_SIZE;
			new_player_pos.dir = DOWN;
		}

		//if (new_player_pos.x + 0.5 > 0 && new_player_pos.x + 0.5 < game_map.tiles.at(0).size() 
		//	&& new_player_pos.y + 0.5 > 0 && new_player_pos.y + 0.5 < game_map.tiles.size())

			if (game_map.tiles.at(floor(new_player_pos.y + new_player_pos.hv)).at(floor(new_player_pos.x + new_player_pos.vv)) != '#')
			{
				if (signbit(hv))
				{
					new_player_pos.x -= mspeed;
				}
				else new_player_pos.x += mspeed;
				if (signbit(vv))
				{
					new_player_pos.y -= mspeed;
				}
				else new_player_pos.y += mspeed;
				if (new_player_pos.x > 0 && new_player_pos.x + TILE_SIZE < game_map.tiles.at(0).size()
					&& new_player_pos.y > 0 && new_player_pos.y + TILE_SIZE < game_map.tiles.size()) player = new_player_pos;
				
			}
		SDL_SetRenderDrawColor(renderer, 10, 0, 0, 255);

		SDL_RenderClear(renderer);
		draw_map(renderer, papryka_texture, game_map);
		draw_player(renderer, player_texture, player);

		SDL_RenderPresent(renderer); // draw frame to screen

		this_thread::sleep_until(current_time = current_time + dt);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);



	SDL_Quit();
	return 0;
}