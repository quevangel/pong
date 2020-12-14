#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL2/SDL.h>

SDL_Window* window;
SDL_Renderer* renderer;
const int window_width = 800;
const int window_height = 800;
inline void init_graphics();


const float frame_time_slice = 1.0f / 60.0f;
struct {
  float position[2];
  float velocity[2];
} ball;
struct player_state {
  int x;
  float y;
  float y_velocity;
  float length;
};
struct player_state left_player, right_player;
inline void init_game_state();
inline void move_player(struct player_state*, int direction);
inline void do_move(struct player_state*);
inline void render_player(struct player_state* state);
inline void render_ball();
inline void move_ball();

int
main(int argc, char* argv[])
{
  init_graphics();
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  init_game_state();

  int num_keys;
  const Uint8* key_state = SDL_GetKeyboardState(&num_keys);
  int left_score = 0, right_score = 0;
  int time_to_appear = -1;
  int who_scored = -1;
  bool in_play = true;
  
  while(true)
    {
      if (time_to_appear == 0)
	{
	  ball.position[0] = window_width * 0.5f;
	  ball.position[1] = window_height * 0.5f;
	  ball.velocity[0] *= -1;
	  in_play = true;
	}
      time_to_appear--;
      if (time_to_appear < 0) time_to_appear = -1;
      
      SDL_Event event;
      while(SDL_PollEvent(&event))
	{
	  switch(event.type)
	    {
	    case SDL_QUIT:
	      goto end_of_main_loop;
	    }
	}
      if (in_play)
	{
	  if (key_state[SDL_SCANCODE_W])
	    move_player(&left_player, -1);
	  else if (key_state[SDL_SCANCODE_S])
	    move_player(&left_player, +1);
	  do_move(&left_player);
      
	  if (key_state[SDL_SCANCODE_I])
	    move_player(&right_player, -1);
	  else if (key_state[SDL_SCANCODE_K])
	    move_player(&right_player, +1);
	  do_move(&right_player);
      
	  move_ball();
	  if (ball.position[0] > window_width && in_play)
	    {
	      left_score++;
	      time_to_appear = 400;
	      who_scored = 0;
	      in_play = false;
	    }
	  else if (ball.position[0] < 0 && in_play)
	    {
	      right_score++;
	      time_to_appear = 400;
	      who_scored = 1;
	      in_play = false;
	    }
	}
      
      SDL_SetRenderDrawColor(renderer, 0, 0, 64, 255);
      SDL_RenderClear(renderer);
      SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      render_player(&right_player);
      SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
      render_player(&left_player);
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      render_ball();
      if (time_to_appear > 0)
      {
	int alpha = 255 * time_to_appear / 400;
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = window_width;
	rect.h = window_height;
	if (who_scored == 0)
	  {
	    SDL_SetRenderDrawColor(renderer, 0, 255, 0, alpha);
	  }
	else
	  {
	    SDL_SetRenderDrawColor(renderer, 255, 0, 0, alpha);
	  }
	SDL_RenderFillRect(renderer, &rect);
      }
      SDL_RenderPresent(renderer);

    }
 end_of_main_loop:;
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  return 0;
}

void init_graphics()
{
  window = SDL_CreateWindow("retro-redo: pong",
			    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			    window_width, window_height,

			    SDL_WINDOW_SHOWN);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void init_game_state()
{
  // left player
  left_player.x = 50;
  left_player.y = window_height * 0.5f;
  left_player.y_velocity = 0.0f;
  left_player.length = 100.0f;
  // right player
  right_player.x = window_width - 50;
  right_player.y = window_height * 0.5f;
  right_player.y_velocity = 0.0f;
  right_player.length = 100.0f;
  // ball
  ball.position[0] = window_width * 0.5f;
  ball.position[1] = window_height * 0.5f;
  ball.velocity[0] = 10.0f;
  ball.velocity[1] = 10.0f;
}

void move_player(struct player_state* state, int direction)
{
  assert(direction == 1 || direction == -1);
  const float velocity_delta = 0.03f;
  state->y_velocity += velocity_delta * direction;
}

void do_move(struct player_state* state)
{
  state->y += state->y_velocity * frame_time_slice;
  if (state->y + 0.5f * state->length > window_width)
    {
      state->y = window_width - 0.5f * state->length;
      state->y_velocity *= -0.5f;
    }
  if (state->y - 0.5f * state->length < 0)
    {
      state->y = 0.5f * state->length;
      state->y_velocity *= -0.5f;
    }
}

void move_ball()
{
  float time_collides_left = (left_player.x - ball.position[0]) / ball.velocity[0];
  float time_collides_right = (right_player.x - ball.position[0]) / ball.velocity[0];
  float time_collides_up = (0 - ball.position[1]) / ball.velocity[1];
  float time_collides_down = (window_height - ball.position[1]) / ball.velocity[1];
  const float contrib_player = 0.5f;
  const float contrib_ball = 0.5f;
#define in_time(x) x > 0.0f && x < frame_time_slice
  if (in_time(time_collides_left))
    {
      float hypothetical_y = ball.position[1] + ball.velocity[1] * time_collides_left;
      if (hypothetical_y >= left_player.y - left_player.length * 0.5f &&
	  hypothetical_y <= left_player.y + left_player.length * 0.5f)
	{
	  ball.position[0] = left_player.x;
	  ball.position[1] = hypothetical_y;
	  ball.velocity[0] *= -1;
	  ball.velocity[1] = contrib_player * left_player.y_velocity + contrib_ball * ball.velocity[1];
	  return;
	}
    }
  if (in_time(time_collides_right))
    {
      fflush(stdout);
      float hypothetical_y = ball.position[1] + ball.velocity[1] * time_collides_right;
      if (hypothetical_y >= right_player.y - left_player.length * 0.5f &&
	  hypothetical_y <= right_player.y + left_player.length * 0.5f)
	{
	  ball.position[0] = right_player.x;
	  ball.position[1] = hypothetical_y;
	  ball.velocity[0] *= -1;
	  ball.velocity[1] = contrib_player * right_player.y_velocity + contrib_ball * ball.velocity[1];
	  return;
	}
    }
  if (in_time(time_collides_up))
    {
      ball.position[1] = 0.0f;
      ball.position[0] += ball.velocity[0] * frame_time_slice;
      ball.velocity[1] *= -1;
      return;
    }
  if (in_time(time_collides_down))
    {
      ball.position[1] = window_height;
      ball.position[0] += ball.velocity[0] * frame_time_slice;
      ball.velocity[1] *= -1;
      return;
    }
  ball.position[0] += ball.velocity[0] * frame_time_slice;
  ball.position[1] += ball.velocity[1] * frame_time_slice;
}

void render_player(struct player_state* state)
{
  SDL_Rect rect;
  rect.x = state->x - 10;
  rect.y = state->y - state->length * 0.5f;
  rect.h = state->length;
  rect.w = 20;
  SDL_RenderFillRect(renderer, &rect);
}

void render_ball()
{
  SDL_Rect rect;
  rect.x = ball.position[0] - 5;
  rect.y = ball.position[1] - 5;
  rect.w = 10;
  rect.h = 10;
  SDL_RenderFillRect(renderer, &rect);
}
