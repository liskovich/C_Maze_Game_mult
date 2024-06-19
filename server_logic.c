#include "server_logic.h"
#include "helpers_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define SHARED_MEM_Q_STATUS "/queue_status_global"
#define SHARED_MEM_Q_STATUS_SIZE sizeof(struct QueueStatus)

#define SHARED_MEM_INIT_LOC "/init_locations_global"
#define SHARED_MEM_INIT_LOC_SIZE sizeof(struct InitialLocations)

#define SHARED_MEM_GAME_STATE "/game_state_global"
#define SHARED_MEM_GAME_STATE_SIZE sizeof(struct GameState)

#define SHARED_MEM_SCOREBOARD "/scoreboard_global"
#define SHARED_MEM_SCOREBOARD_SIZE sizeof(struct GameState)

/*
TODO: replace this for your own map
NOTE: the dimensions of map should be GAME_MAP_WIDTH * GAME_MAP_HEIGHT
*/
#define SELECTED_GAME_MAP "sample_map_1.txt"
#define PLAYER_MOVE_STEP 0.3f

struct QueueStatus *q_status_GLOBAL;
struct InitialLocations *init_locations_GLOBAL;
struct GameState *game_state_GLOBAL;
struct Scoreboard *scoreboard_GLOBAL;

int32_t current_state_GLOBAL;
int32_t player_count_GLOBAL;

int32_t win_score_required_GLOBAL;

/* data for object spawning */
int position_count = 5;
CoordinateTuple obj1_positions[5];
CoordinateTuple obj2_positions[5];
CoordinateTuple obj3_positions[5];

void init_random_object_positions();

CoordinateTuple retrieve_random_object_coordinates(int object_id)
{
  /* get random coordinates for object */
  srand(time(NULL));
  int random_idx = rand() % position_count;

  if (object_id == 1)
  {
    return obj1_positions[random_idx];
  }
  else if (object_id == 2)
  {
    return obj2_positions[random_idx];
  }
  else
  {
    return obj3_positions[random_idx];
  }
}

void initGameServerLogic(int32_t win_score_required)
{
  /* set the current_state_GLOBAL to some default */
  current_state_GLOBAL = QUEUE_LOGIC_STATE;
  init_random_object_positions();

  /* set the player_count_GLOBAL to 0 */
  player_count_GLOBAL = 0;

  /* set the win_score_required_GLOBAL to win_score_required */
  win_score_required_GLOBAL = win_score_required;
}

/*
queue operations
*/

/* called by initGameServerLogic */
void initQueue(int32_t max_player_count)
{
  int shm_fd;

  /* create a shared memory */
  shm_fd = shm_open(SHARED_MEM_Q_STATUS, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1)
  {
    printf("Failed to create shared memory.\n");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, SHARED_MEM_Q_STATUS_SIZE) == -1)
  {
    printf("Failed to truncate shared memory.\n");
    exit(EXIT_FAILURE);
  }

  /* memory for the queue status for max_player_count players */
  q_status_GLOBAL = (struct QueueStatus *)mmap(NULL, SHARED_MEM_Q_STATUS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
  if (q_status_GLOBAL == MAP_FAILED)
  {
    printf("Failed to map memory for QueueStatus.\n");
    exit(EXIT_FAILURE);
  }
  q_status_GLOBAL->players_count = max_player_count;

  /* memory for the array of queue players */
  q_status_GLOBAL->players_in_queue = (struct PlayerQueueInfo **)malloc(max_player_count * sizeof(struct PlayerQueueInfo *));
  if (q_status_GLOBAL->players_in_queue == NULL)
  {
    free(q_status_GLOBAL);
    exit(EXIT_FAILURE);
  }

  /* memory for each queue player */
  int i;
  for (i = 0; i < max_player_count; i++)
  {
    q_status_GLOBAL->players_in_queue[i] = NULL;
  }
  printf("Queue state initialized successfully...\n");
}

/* called by transitionToGameState */
void deleteQueue()
{
  /* free the content of q_status_GLOBAL */
  int i;
  for (i = 0; i < q_status_GLOBAL->players_count; i++)
  {
    if (q_status_GLOBAL->players_in_queue[i] != NULL)
    {
      free(q_status_GLOBAL->players_in_queue[i]);
    }
  }
  free(q_status_GLOBAL->players_in_queue);

  /* delete shared memory */
  if (munmap(q_status_GLOBAL, SHARED_MEM_Q_STATUS_SIZE) == -1)
  {
    printf("Failed to unmap memory.\n");
    exit(EXIT_FAILURE);
  }
  if (shm_unlink(SHARED_MEM_Q_STATUS) == -1)
  {
    printf("Failed to unlink shared memory.\n");
    exit(EXIT_FAILURE);
  }

  printf("Freed queue state successfully...\n");
}

struct QueueStatus *getCurrentQueueStatus()
{
  return q_status_GLOBAL;
}

/* directly after this, getCurrentQueueStatus should be called to see changes */
int32_t addPlayerToQueue(char *player_username)
{
  printf("Inside addPlayerToQueue.\n");

  if (q_status_GLOBAL == NULL)
  {
    printf("q_status_GLOBAL is uninitialized.\n");
    return -1;
  }

  printf("q_status_GLOBAL is accessible.\n");
  printf("q_status_GLOBAL players count: %d (intermediate check).\n", q_status_GLOBAL->players_count);

  /* find a free memory spot for the player (error if not found) */
  int32_t available_spot_index = -1;
  int i;
  for (i = 0; i < q_status_GLOBAL->players_count; i++)
  {
    if (q_status_GLOBAL->players_in_queue[i] == NULL)
    {
      available_spot_index = i;
      break;
    }
  }
  if (available_spot_index == -1)
  {
    return -1;
  }

  /* memory for the player */
  printf("Allocating player spot...\n");
  q_status_GLOBAL->players_in_queue[available_spot_index] = (struct PlayerQueueInfo *)malloc(sizeof(struct PlayerQueueInfo));
  if (q_status_GLOBAL->players_in_queue[available_spot_index] == NULL)
  {
    printf("Failed to alloc memory for the player.\n");
    return -1;
  }

  /* set player information */
  printf("Setting player info...\n");
  int32_t player_id = available_spot_index + 1;
  strcpy(q_status_GLOBAL->players_in_queue[available_spot_index]->username, player_username);
  q_status_GLOBAL->players_in_queue[available_spot_index]->gamer_id = player_id;
  q_status_GLOBAL->players_in_queue[available_spot_index]->is_ready = 0;

  /* update player count */
  player_count_GLOBAL++;
  printf("Current queue player count: %d.\n", player_count_GLOBAL);

  return player_id;
}

/* directly after this, getCurrentQueueStatus should be called to see changes */
int32_t markPlayerReady(int32_t player_id)
{
  if (q_status_GLOBAL == NULL)
  {
    return -1;
  }

  /* find player with id equal to player_id in q_status_GLOBAL */
  int i;
  for (i = 0; i < q_status_GLOBAL->players_count; i++)
  {
    if (q_status_GLOBAL->players_in_queue[i] != NULL && q_status_GLOBAL->players_in_queue[i]->gamer_id == player_id)
    {
      q_status_GLOBAL->players_in_queue[i]->is_ready = 1;

      /* check if all players in queue are ready */
      int all_players_ready = 1;
      int j;
      for (j = 0; j < q_status_GLOBAL->players_count; j++)
      {
        if (q_status_GLOBAL->players_in_queue[j] != NULL && q_status_GLOBAL->players_in_queue[j]->is_ready == 0)
        {
          all_players_ready = 0;
          break;
        }
      }

      /* if all players in queue are ready, transition to game state */
      if (all_players_ready)
      {
        transitionToGameState();
        /* specific code for "all players ready" */
        return -3;
      }
      return player_id;
    }
  }
  return -1;
}

/* directly after this, getCurrentQueueStatus should be called to see changes */
int32_t removePlayerFromQueue(int32_t player_id)
{
  /* find player with id equal to player_id in q_status_GLOBAL */
  int i;
  for (i = 0; i < q_status_GLOBAL->players_count; i++)
  {
    if (q_status_GLOBAL->players_in_queue[i] != NULL && q_status_GLOBAL->players_in_queue[i]->gamer_id == player_id)
    {
      /* free player's memory */
      free(q_status_GLOBAL->players_in_queue[i]);
      q_status_GLOBAL->players_in_queue[i] = NULL;

      /* update player count */
      player_count_GLOBAL--;
      printf("Current queue player count: %d.\n", player_count_GLOBAL);

      return i;
    }
  }
  return -1;
}

/*
game operations
*/

/* called by transitionToGameState */
void initGameMapLocations(int32_t width, int32_t height)
{
  int shm_fd;

  /* create a shared memory */
  shm_fd = shm_open(SHARED_MEM_INIT_LOC, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1)
  {
    printf("Failed to create shared memory.\n");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, SHARED_MEM_INIT_LOC_SIZE) == -1)
  {
    printf("Failed to truncate shared memory.\n");
    exit(EXIT_FAILURE);
  }

  /* memory for the initial locations */
  init_locations_GLOBAL = (struct InitialLocations *)mmap(NULL, SHARED_MEM_INIT_LOC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
  if (init_locations_GLOBAL == MAP_FAILED)
  {
    printf("Failed to map memory for InitialLocations.\n");
    exit(EXIT_FAILURE);
  }

  /* add values to shared memory */
  init_locations_GLOBAL->map_width = width;
  init_locations_GLOBAL->map_height = height;
  init_locations_GLOBAL->tiles = (char *)malloc(width * height * sizeof(char));
  if (init_locations_GLOBAL->tiles == NULL)
  {
    printf("Failed to alloc memory for map tiles.\n");
    exit(EXIT_FAILURE);
  }

  /* build map with passable and non-passable tiles */
  /* passable ('.'), non-passable ('|') */
  read_map_from_file(SELECTED_GAME_MAP);
}

/* called by transitionToBoardState */
void deleteGameMapLocations()
{
  /* free memory for tiles */
  free(init_locations_GLOBAL->tiles);

  /* delete shared memory */
  if (munmap(init_locations_GLOBAL, SHARED_MEM_INIT_LOC_SIZE) == -1)
  {
    printf("Failed to unmap memory.\n");
    exit(EXIT_FAILURE);
  }
  if (shm_unlink(SHARED_MEM_INIT_LOC) == -1)
  {
    printf("Failed to unlink shared memory.\n");
    exit(EXIT_FAILURE);
  }

  printf("Freed initial locations successfully...\n");
}

struct InitialLocations *getCurrentMapLocations()
{
  return init_locations_GLOBAL;
}

/* called by transitionToGameState */
void initGameState(int32_t player_count)
{
  int shm_fd;

  /* create a shared memory */
  shm_fd = shm_open(SHARED_MEM_GAME_STATE, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1)
  {
    printf("Failed to create shared memory.\n");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, SHARED_MEM_GAME_STATE_SIZE) == -1)
  {
    printf("Failed to truncate shared memory.\n");
    exit(EXIT_FAILURE);
  }

  /* memory for the game state for player_count players */
  game_state_GLOBAL = (struct GameState *)mmap(NULL, SHARED_MEM_GAME_STATE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
  if (game_state_GLOBAL == MAP_FAILED)
  {
    printf("Failed to map memory for GameState.\n");
    exit(EXIT_FAILURE);
  }
  game_state_GLOBAL->players_count = player_count;
  game_state_GLOBAL->objects_count = GAME_OBJECTS_ON_MAP;

  /* memory for the array of game players */
  game_state_GLOBAL->players = (struct PlayerGameInfo **)malloc(player_count * sizeof(struct PlayerGameInfo *));
  if (game_state_GLOBAL->players == NULL)
  {
    free(game_state_GLOBAL);
    exit(EXIT_FAILURE);
  }

  /* memory for each game player */
  int i;
  for (i = 0; i < player_count; i++)
  {
    game_state_GLOBAL->players[i] = NULL;
  }

  /* copy player details from q_status_GLOBAL to game_state_GLOBAL */
  int game_state_idx = 0;
  for (i = 0; i < q_status_GLOBAL->players_count; i++)
  {
    /* find "real" players in the players_in_queue */
    if (q_status_GLOBAL->players_in_queue[i] != NULL &&
        q_status_GLOBAL->players_in_queue[i]->gamer_id >= 0 &&
        strcmp(q_status_GLOBAL->players_in_queue[i]->username, "") != 0)
    {
      /* init a new PlayerGameInfo */
      struct PlayerGameInfo *new_player = (struct PlayerGameInfo *)malloc(sizeof(struct PlayerGameInfo));
      if (new_player == NULL)
      {
        printf("Failed to alloc memory for player in the game state.\n");
        return;
      }

      /* copy data from the player in the queue to player in game state */
      memcpy(new_player->username, q_status_GLOBAL->players_in_queue[i]->username, 32 * sizeof(char));
      new_player->gamer_id = q_status_GLOBAL->players_in_queue[i]->gamer_id;

      /*
      TODO: check that coordinates do not end up on non-passable tiles
      */
      new_player->x = 1.6f; /* 0.4f */
      new_player->y = 1.9f; /* 0.7f */

      new_player->score = 0.0f;

      /* add player to game_state_GLOBAL->players */
      game_state_GLOBAL->players[game_state_idx++] = new_player;
    }
  }

  /* memory for the array of game objects */
  game_state_GLOBAL->objects = (struct ObjectInfo **)malloc(game_state_GLOBAL->objects_count * sizeof(struct ObjectInfo *));
  if (game_state_GLOBAL->objects == NULL)
  {
    free(game_state_GLOBAL);
    exit(EXIT_FAILURE);
  }

  /* memory for each game object */
  for (i = 0; i < game_state_GLOBAL->objects_count; i++)
  {
    game_state_GLOBAL->objects[i] = NULL;
  }

  /* add GAME_OBJECTS_ON_MAP objects with random coordinates and type */
  for (i = 0; i < game_state_GLOBAL->objects_count; i++)
  {
    /* init a new ObjectInfo */
    struct ObjectInfo *new_object = (struct ObjectInfo *)malloc(sizeof(struct ObjectInfo));
    if (new_object == NULL)
    {
      printf("Failed to alloc memory for object in the game state.\n");
      return;
    }

    /* type 'c' means a coin */
    /*
    TODO: introduce other object types
    */
    new_object->type = 'c';
    new_object->id = i + 1;

    CoordinateTuple obj_coord = retrieve_random_object_coordinates(new_object->id);
    new_object->x = obj_coord.x; /* 0.0f */
    new_object->y = obj_coord.y; /* 0.0f */

    /* add object to game_state_GLOBAL->objects */
    game_state_GLOBAL->objects[i] = new_object;
  }

  printf("Game state initialized successfully...\n");
}

/* called by transitionToBoardState */
void deleteGameState()
{
  /* free the content of game_state_GLOBAL */
  int i;
  for (i = 0; i < game_state_GLOBAL->players_count; i++)
  {
    if (game_state_GLOBAL->players[i] != NULL)
    {
      free(game_state_GLOBAL->players[i]);
    }
  }
  for (i = 0; i < game_state_GLOBAL->objects_count; i++)
  {
    if (game_state_GLOBAL->objects[i] != NULL)
    {
      free(game_state_GLOBAL->objects[i]);
    }
  }
  free(game_state_GLOBAL->players);
  free(game_state_GLOBAL->objects);

  /* delete shared memory */
  if (munmap(game_state_GLOBAL, SHARED_MEM_GAME_STATE_SIZE) == -1)
  {
    printf("Failed to unmap memory.\n");
    exit(EXIT_FAILURE);
  }
  if (shm_unlink(SHARED_MEM_GAME_STATE) == -1)
  {
    printf("Failed to unlink shared memory.\n");
    exit(EXIT_FAILURE);
  }

  printf("Freed game state successfully...\n");
}

struct GameState *getCurrentGameState()
{
  return game_state_GLOBAL;
}

/* directly after this, getCurrentGameState should be called to see changes */
void handlePlayerMove(int32_t player_id, char *pressed_key)
{
  if (game_state_GLOBAL == NULL)
  {
    printf("Game state is uninitialized.\n");
    return;
  }

  /* find player with id equal to player_id in game_state_GLOBAL */
  struct PlayerGameInfo *active_player = NULL;
  int i;
  for (i = 0; i < game_state_GLOBAL->players_count; i++)
  {
    if (game_state_GLOBAL->players[i]->gamer_id == player_id)
    {
      active_player = game_state_GLOBAL->players[i];
      break;
    }
  }
  if (active_player == NULL)
  {
    printf("The player with id: %d was not found.\n", player_id);
    return;
  }

  /*
  TODO: check if update player's x and y coordinates according to pressed_key will not end up in a wall
  requires checking against the init_locations_GLOBAL->tiles
  */

  /* update the player's x and/or y coordinates according to pressed_key */
  if (strcmp(pressed_key, "w") == 0)
  {
    /* move up */
    float player_new_y_coord = active_player->y + PLAYER_MOVE_STEP;

    if (canMoveToCoordinate(active_player->x, player_new_y_coord))
    {
      active_player->y += PLAYER_MOVE_STEP;
    }
  }
  else if (strcmp(pressed_key, "s") == 0)
  {
    /* move down */
    float player_new_y_coord = active_player->y - PLAYER_MOVE_STEP;

    if (canMoveToCoordinate(active_player->x, player_new_y_coord))
    {
      active_player->y -= PLAYER_MOVE_STEP;
    }
  }
  else if (strcmp(pressed_key, "a") == 0)
  {
    /* move left */
    float player_new_x_coord = active_player->x - PLAYER_MOVE_STEP;

    if (canMoveToCoordinate(player_new_x_coord, active_player->y))
    {
      active_player->x -= PLAYER_MOVE_STEP;
    }
  }
  else if (strcmp(pressed_key, "d") == 0)
  {
    /* move right */
    float player_new_x_coord = active_player->x + PLAYER_MOVE_STEP;

    if (canMoveToCoordinate(player_new_x_coord, active_player->y))
    {
      active_player->x += PLAYER_MOVE_STEP;
    }
  }

  /* check if player's x and y coordinates end up on an object (if yes, collect it using handleObjectCollection) */
  handleObjectCollection(player_id);

  /* check if player's score reached the win_score_required_GLOBAL (if yes, call handleGameOver) */
  if (active_player->score >= win_score_required_GLOBAL)
  {
    handleGameOver();
  }
}

/*
board operations
*/

/* called by transitionToBoardState */
void initScoreboard()
{
  int shm_fd;

  /* create a shared memory */
  shm_fd = shm_open(SHARED_MEM_SCOREBOARD, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1)
  {
    printf("Failed to create shared memory.\n");
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, SHARED_MEM_SCOREBOARD_SIZE) == -1)
  {
    printf("Failed to truncate shared memory.\n");
    exit(EXIT_FAILURE);
  }

  /* memory for the scoreboard for player_count players */
  scoreboard_GLOBAL = (struct Scoreboard *)mmap(NULL, SHARED_MEM_SCOREBOARD_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, shm_fd, 0);
  if (scoreboard_GLOBAL == MAP_FAILED)
  {
    printf("Failed to map memory for Scoreboard.\n");
    exit(EXIT_FAILURE);
  }
  scoreboard_GLOBAL->players_count = game_state_GLOBAL->players_count;

  /* alloc memory for scoreboard player list */
  scoreboard_GLOBAL->players = (struct PlayerScoreboardInfo **)malloc(game_state_GLOBAL->players_count * sizeof(struct PlayerScoreboardInfo *));
  if (scoreboard_GLOBAL->players == NULL)
  {
    printf("Failed to allocate scoreboard player list.\n");
    exit(EXIT_FAILURE);
  }

  /* copy player details from game_state_GLOBAL to scoreboard_GLOBAL */
  /*
  TODO: sort players by score descending
  */
  int i;
  for (i = 0; i < game_state_GLOBAL->players_count; i++)
  {
    /* memory for each player in scoreboard */
    scoreboard_GLOBAL->players[i] = (struct PlayerScoreboardInfo *)malloc(sizeof(struct PlayerScoreboardInfo));
    if (scoreboard_GLOBAL->players[i] == NULL)
    {
      printf("Failed to allocate memory for a scoreboard player.\n");
      exit(EXIT_FAILURE);
    }

    /* copy player data from game_state_GLOBAL to scoreboard_GLOBAL */
    memcpy(scoreboard_GLOBAL->players[i]->username, game_state_GLOBAL->players[i]->username, 32 * sizeof(char));
    scoreboard_GLOBAL->players[i]->gamer_id = game_state_GLOBAL->players[i]->gamer_id;
    scoreboard_GLOBAL->players[i]->score = game_state_GLOBAL->players[i]->score;

    /* add stats (score for now) tp each player's stats info */
    /*
    TODO: add more stats
    */
    scoreboard_GLOBAL->players[i]->stats_count = 1;
    scoreboard_GLOBAL->players[i]->stats_info = (char *)malloc(scoreboard_GLOBAL->players[i]->stats_count * 128 * sizeof(char));
    if (scoreboard_GLOBAL->players[i]->stats_info == NULL)
    {
      printf("Failed to allocate memory for a scoreboard player's stats.\n");
      exit(EXIT_FAILURE);
    }
    snprintf(scoreboard_GLOBAL->players[i]->stats_info, 128, "Score: %.2f", scoreboard_GLOBAL->players[i]->score);
  }
}

/* called when server is killed */
void deleteScoreboard()
{
  /* free memory for each player in the scoreboard */
  int i;
  for (i = 0; i < scoreboard_GLOBAL->players_count; i++)
  {
    if (scoreboard_GLOBAL->players[i] != NULL)
    {
      /* free memory for each player's stats info in the scoreboard */
      free(scoreboard_GLOBAL->players[i]->stats_info);
      free(scoreboard_GLOBAL->players[i]);
    }
  }
  free(scoreboard_GLOBAL->players);

  /* delete shared memory */
  if (munmap(scoreboard_GLOBAL, SHARED_MEM_SCOREBOARD_SIZE) == -1)
  {
    printf("Failed to unmap memory.\n");
    exit(EXIT_FAILURE);
  }
  if (shm_unlink(SHARED_MEM_SCOREBOARD) == -1)
  {
    printf("Failed to unlink shared memory.\n");
    exit(EXIT_FAILURE);
  }

  printf("Freed scoreboard successfully...\n");
}

struct Scoreboard *getCurrentScoreboard()
{
  return scoreboard_GLOBAL;
}

/*
common
*/

void transitionToGameState()
{
  /* update the current_state_GLOBAL */
  current_state_GLOBAL = GAME_LOGIC_STATE;

  /* call initGameMapLocations */
  initGameMapLocations(GAME_MAP_WIDTH, GAME_MAP_HEIGHT);

  /* call initGameState */
  int32_t real_players_count = 0;
  if (q_status_GLOBAL == NULL)
  {
    printf("Cannot retrieve data about queue state...\n");
    return;
  }

  /* check the number of actual players in players_in_queue */
  int i;
  for (i = 0; i < q_status_GLOBAL->players_count; i++)
  {
    if (q_status_GLOBAL->players_in_queue[i] != NULL &&
        q_status_GLOBAL->players_in_queue[i]->gamer_id >= 0 &&
        strcmp(q_status_GLOBAL->players_in_queue[i]->username, "") != 0)
    {
      real_players_count++;
    }
  }
  initGameState(real_players_count);

  /* call deleteQueue */
  deleteQueue();
}

void transitionToBoardState()
{
  /* update the current_state_GLOBAL */
  current_state_GLOBAL = BOARD_LOGIC_STATE;

  /* call initScoreboard */
  initScoreboard();

  /* call deleteGameMapLocations and deleteGameState */
  /*
  deleteGameMapLocations();
  deleteGameState();
  */
}

int32_t getCurrentState()
{
  return current_state_GLOBAL;
}

/*
internal logic
*/

void init_random_object_positions()
{
  /* obj1_positions */
  obj1_positions[0].x = 0.9f;
  obj1_positions[0].y = 12.9f;

  obj1_positions[1].x = 1.1f;
  obj1_positions[1].y = 3.9f;

  obj1_positions[2].x = 1.0f;
  obj1_positions[2].y = 6.9f;

  obj1_positions[3].x = 12.8f;
  obj1_positions[3].y = 12.0f;

  obj1_positions[4].x = 10.7f;
  obj1_positions[4].y = 6.0f;

  /* obj2_positions */
  obj2_positions[0].x = 0.9f;
  obj2_positions[0].y = 10.8f;

  obj2_positions[1].x = 3.0f;
  obj2_positions[1].y = 3.9f;

  obj2_positions[2].x = 6.0f;
  obj2_positions[2].y = 10.8f;

  obj2_positions[3].x = 11.4f;
  obj2_positions[3].y = 8.7f;

  obj2_positions[4].x = 10.7f;
  obj2_positions[4].y = 0.9f;

  /* obj3_positions */
  obj3_positions[0].x = 2.1f;
  obj3_positions[0].y = 5.7f;

  obj3_positions[1].x = 5.1f;
  obj3_positions[1].y = 1.8f;

  obj3_positions[2].x = 7.2f;
  obj3_positions[2].y = 12.9f;

  obj3_positions[3].x = 8.7f;
  obj3_positions[3].y = 9.9f;

  obj3_positions[4].x = 12.8f;
  obj3_positions[4].y = 3.0f;
}

int isDifferenceLessThanOrEqual(float a, float b)
{
  float difference = fabs(a - b);
  /*
  TODO: adjust difference calculation for higher precision
  */
  if (difference <= 0.7f)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void handleObjectCollection(int32_t player_id)
{
  /* find player with id equal to player_id in game_state_GLOBAL */
  struct PlayerGameInfo *active_player = NULL;
  int i;
  for (i = 0; i < game_state_GLOBAL->players_count; i++)
  {
    if (game_state_GLOBAL->players[i]->gamer_id == player_id)
    {
      active_player = game_state_GLOBAL->players[i];
      break;
    }
  }
  if (active_player == NULL)
  {
    printf("The player with id: %d was not found.\n", player_id);
    return;
  }

  /* retrieve all objects in game_state_GLOBAL */
  for (i = 0; i < game_state_GLOBAL->objects_count; i++)
  {
    /* compare object coordinates with player corrdinates */
    int x_diff = isDifferenceLessThanOrEqual(game_state_GLOBAL->objects[i]->x, active_player->x);
    int y_diff = isDifferenceLessThanOrEqual(game_state_GLOBAL->objects[i]->y, active_player->y);

    /* check if player has stepped on one of objects */
    if (x_diff == 1 && y_diff == 1)
    {
      /* update the player's score accordingly (TODO: based on the objects type) */
      active_player->score += SCORE_INCREASE;

      /* update the objects's x and y coordinates and/or type (randomly) */
      CoordinateTuple obj_new_coord = retrieve_random_object_coordinates(game_state_GLOBAL->objects[i]->id);
      game_state_GLOBAL->objects[i]->x = obj_new_coord.x;
      game_state_GLOBAL->objects[i]->y = obj_new_coord.y;
      break;
    }
  }
}

void handleGameOver()
{
  /* call transitionToBoardState */
  transitionToBoardState();
}

void read_map_from_file(const char *filename)
{
  if (init_locations_GLOBAL == NULL)
  {
    printf("InitialLocations has to be initialized first.\n");
    return;
  }

  /* open the tiles file for reading */
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    perror("Error opening file");
    return;
  }

  int total_tiles_size = init_locations_GLOBAL->map_width * init_locations_GLOBAL->map_height;

  /* read the tiles file character by character */
  int i;
  for (i = 0; i < total_tiles_size; i++)
  {
    int ch = fgetc(file);

    /* avoid non-necessary characters */
    if (ch == EOF)
    {
      break;
    }
    if (ch == '\n')
    {
      i--;
      continue;
    }

    /* copy character into the tiles array of init_locations_GLOBAL */
    init_locations_GLOBAL->tiles[i] = (char)ch;
  }
  fclose(file);
}

void getTileCenter(int32_t row, int32_t col, float *x, float *y)
{
  float tileWidth = 15.0f / init_locations_GLOBAL->map_width;
  float tileHeight = 15.4f / init_locations_GLOBAL->map_height;

  float tileHalfWidth = tileWidth / 2.0f;
  float tileHalfHeight = tileHeight / 2.0f;

  *x = col * tileWidth + tileHalfWidth;
  *y = row * tileHeight + tileHalfHeight;
}

int32_t canMoveToCoordinate(float x, float y)
{
  /* calculate row and column of tile the player is currently in */
  int tileRow = (int)floor((y / 15.4f) * init_locations_GLOBAL->map_height);
  int tileCol = (int)floor((x / 15.0f) * init_locations_GLOBAL->map_width);

  /* check if player is within the maze */
  if (tileRow >= 0 && tileRow < init_locations_GLOBAL->map_height && tileCol >= 0 && tileCol < init_locations_GLOBAL->map_width)
  {
    float tileCenterX, tileCenterY;
    getTileCenter(tileRow, tileCol, &tileCenterX, &tileCenterY);

    /* check if player is close to center of tile */
    float threshold = TILE_SIZE / 4.0f;
    if (fabs(x - tileCenterX) < threshold && fabs(y - tileCenterY) < threshold)
    {
      /* return init_locations_GLOBAL->tiles[tileRow * init_locations_GLOBAL->map_width + tileCol] == '.'; */
      return 1;
    }
  }

  return 1;
}