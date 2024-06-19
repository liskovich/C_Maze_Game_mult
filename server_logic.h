#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <stdint.h>

#define QUEUE_LOGIC_STATE 1
#define GAME_LOGIC_STATE 2
#define BOARD_LOGIC_STATE 3

#define GAME_MAP_WIDTH 15
#define GAME_MAP_HEIGHT 15

#define TILE_SIZE 2.0f

#define GAME_OBJECTS_ON_MAP 3
#define SCORE_INCREASE 20

typedef struct
{
  float x;
  float y;
} CoordinateTuple;

void initGameServerLogic(int32_t win_score_required);

/* queue operations */
void initQueue(int32_t max_player_count);
void deleteQueue();
struct QueueStatus *getCurrentQueueStatus();

int32_t addPlayerToQueue(char *player_username);
int32_t markPlayerReady(int32_t player_id);
int32_t removePlayerFromQueue(int32_t player_id);

/* game operations */
void initGameMapLocations(int32_t width, int32_t height);
void deleteGameMapLocations();
struct InitialLocations *getCurrentMapLocations();

void initGameState(int32_t player_count);
void deleteGameState();
struct GameState *getCurrentGameState();

void handlePlayerMove(int32_t player_id, char *pressed_key);

/* board operations */
void initScoreboard();
void deleteScoreboard();
struct Scoreboard *getCurrentScoreboard();

/* common */
void transitionToGameState();
void transitionToBoardState();
int32_t getCurrentState();

/* internal only functions */
void handleObjectCollection(int32_t player_id);
void handleGameOver();
void read_map_from_file(const char *filename);

void getTileCenter(int32_t row, int32_t col, float *x, float *y);
int32_t canMoveToCoordinate(float x, float y);

#endif