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
#include <map>

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
	double x, y, xt, yt;
	player_dir_e dir;
	bool stop, push;
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

void draw_map(SDL_Renderer *renderer, std::map<char,SDL_Texture*> textures, const game_map_t &m)
{

	for (unsigned y = 0; y < m.h; y++)
	{
		for (unsigned x = 0; x < m.w; x++)
		{
			SDL_Rect rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
			SDL_RenderCopy(renderer, textures[m.tiles.at(y).at(x)], NULL, &rect);
		}
	}
}

void draw_player(SDL_Renderer *renderer, SDL_Texture *player_texture, const player_t &p, int frame)
{
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect rectr = { TILE_SIZE*((frame/30)%2),
		0,
		TILE_SIZE,
		TILE_SIZE};
	rectr.y = TILE_SIZE*p.dir;	
	if (!p.stop) rectr.x += TILE_SIZE;
	if (!p.stop) rectr.x += 2*TILE_SIZE;
	
	SDL_Rect rect = { (int)(p.x * TILE_SIZE),
		(int)(p.y * TILE_SIZE),
		TILE_SIZE,
		TILE_SIZE };
	/*SDL_RenderFillRect(renderer, &rect);*/
	SDL_RenderCopy(renderer, player_texture, &rectr, &rect);

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
}

int main(int, char **)
{
	using namespace std;
	using namespace std::chrono;
	const int width = 640;
	const int height = 400;
	const int scale = 2;
	errcheck(SDL_Init(SDL_INIT_VIDEO) != 0);

	SDL_Window *window = SDL_CreateWindow(
		"My Next Superawesome Game", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, width*scale, height*scale, SDL_WINDOW_SHOWN);
	errcheck(window == nullptr);

	SDL_Renderer *renderer = SDL_CreateRenderer(
		window, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_PRESENTVSYNC
	errcheck(renderer == nullptr);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
	SDL_RenderSetLogicalSize(renderer, width, height);
	
	SDL_Surface *surface = SDL_LoadBMP("data/player.bmp");
	SDL_SetColorKey(surface,SDL_TRUE,SDL_MapRGB(surface->format,0,0xFF,0xFF));
	SDL_Texture* player_texture = SDL_CreateTextureFromSurface(renderer, surface);
	
	map<char,SDL_Texture*> textures;
	surface = SDL_LoadBMP("data/poziomdzungla/podloga.bmp");
	textures['.'] = SDL_CreateTextureFromSurface(renderer, surface);
	surface = SDL_LoadBMP("data/poziomdzungla/scianadol.bmp");
	textures['#'] = SDL_CreateTextureFromSurface(renderer, surface);	
	
	SDL_FreeSurface(surface);
	surface = NULL;

	// auto dt = 15ms;
	milliseconds dt(15);

	auto game_map = load_map("data/level0");
	player_t player;
	player.x = game_map.start_x;
	player.y = game_map.start_y;
	player.xt = player.x;
	player.yt = player.y;
	debug_print_map(game_map);

	steady_clock::time_point current_time =
		steady_clock::now(); // remember current time
	int frame = 0;
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
		new_player_pos.stop = false;
		/*if (new_player_pos.x > (double)new_player_pos.xt)
		{
			new_player_pos.x -= 0.1;
			//new_player_pos.dir = LEFT;
		}
		else if (new_player_pos.x < (double)new_player_pos.xt)
		{
			new_player_pos.x += 0.1;
			//new_player_pos.dir = RIGHT;
		}
		else if (new_player_pos.y > (double)new_player_pos.yt)
		{
			new_player_pos.y -= 0.1;
			//new_player_pos.dir = UP;
		}
		else if (new_player_pos.y < (double)new_player_pos.yt)
		{
			new_player_pos.y += 0.1;
			//new_player_pos.dir = DOWN;
		}*/
		new_player_pos.x += (player.xt - player.x)*0.1;
		new_player_pos.y += (player.yt - player.y)*0.1;
		if ((abs(new_player_pos.x - new_player_pos.xt) < 0.5)  && (abs(new_player_pos.y - new_player_pos.yt) < 0.5))
		{
			if (state[SDL_SCANCODE_LEFT]) 
			{
				new_player_pos.xt -= 1;
				new_player_pos.dir = LEFT;
			}
			else if (state[SDL_SCANCODE_RIGHT])
			{
				new_player_pos.xt += 1;
				new_player_pos.dir = RIGHT;
			}
			else if (state[SDL_SCANCODE_UP])
			{
				new_player_pos.yt -= 1;
				new_player_pos.dir = UP;
			}
			else if (state[SDL_SCANCODE_DOWN])
			{
				new_player_pos.yt += 1;
				new_player_pos.dir = DOWN;
			}
			else new_player_pos.stop = true;			
		}
		/*else
		{
			if (new_player_pos.x > new_player_pos.xt)
			{
				new_player_pos.x -= 0.1;
				//new_player_pos.dir = LEFT;
			}
			else if (new_player_pos.x < new_player_pos.xt)
			{
				new_player_pos.x += 0.1;
				//new_player_pos.dir = RIGHT;
			}
			else if (new_player_pos.y > new_player_pos.yt)
			{
				new_player_pos.y -= 0.1;
				//new_player_pos.dir = UP;
			}
			else if (new_player_pos.y < new_player_pos.yt)
			{
				new_player_pos.y += 0.1;
				//new_player_pos.dir = DOWN;
			}
		}*/

		//if (new_player_pos.x + 0.5 > 0 && new_player_pos.x + 0.5 < game_map.tiles.at(0).size() 
		//	&& new_player_pos.y + 0.5 > 0 && new_player_pos.y + 0.5 < game_map.tiles.size())

			/*if (game_map.tiles.at(new_player_pos.y).at(new_player_pos.x) != '#'
			&& game_map.tiles.at(new_player_pos.y+1).at(new_player_pos.x) != '#'
			&& game_map.tiles.at(new_player_pos.y+1).at(new_player_pos.x+1) != '#'
			&& game_map.tiles.at(new_player_pos.y).at(new_player_pos.x+1) != '#')
			{*/
				if (new_player_pos.xt > 0 && new_player_pos.xt + 1 < game_map.tiles.at(0).size()
					&& new_player_pos.yt > 0 && new_player_pos.yt + 1 < game_map.tiles.size()) 
				{
					player = new_player_pos;
					player = new_player_pos;
					cout << player.x << " " << player.y << endl;
					cout << player.xt << " " << player.yt << endl;
				}				
			//}
		SDL_SetRenderDrawColor(renderer, 10, 0, 0, 255);

		SDL_RenderClear(renderer);
		draw_map(renderer, textures, game_map);
		draw_player(renderer, player_texture, player,frame);
		frame++;
		SDL_RenderPresent(renderer); // draw frame to screen

		this_thread::sleep_until(current_time = current_time + dt);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);



	SDL_Quit();
	return 0;
}
