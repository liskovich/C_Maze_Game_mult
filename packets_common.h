#ifndef PACKETS_COMMON_H
#define PACKETS_COMMON_H

#include <stdint.h>

#define JOIN_QUEUE_REQ (uint16_t)0
#define JOIN_QUEUE_RESP (uint16_t)1
#define LEAVE_QUEUE_REQ (uint16_t)2
#define GAME_READY_RESP (uint16_t)3
#define PLAYER_READY_REQ (uint16_t)4
#define QUEUE_STATUS (uint16_t)5
#define BTN_PRESSED (uint16_t)6
#define INIT_LOCATIONS (uint16_t)7
#define GAME_STATE (uint16_t)8
#define GAME_OVER (uint16_t)9
#define NOTIFICATION (uint16_t)10
#define SCORE_BOARD (uint16_t)11

#pragma pack(push, 1)
struct Header
{
  char soh;
  int32_t msg_id;
  uint16_t msg_type;
  int32_t size;
  int32_t order;
  int32_t player_id;
  char stx;
};

struct Footer
{
  int32_t checksum;
  char end;
};

/* join state */
struct JoinQueueRequest
{
  char username[32];
};

struct JoinQueueResponse
{
  char success;
};

/* queue state */
struct LeaveQueueRequest
{
};

struct PlayerReadyRequest
{
};

struct PlayerQueueInfo
{
  char username[32];
  int32_t gamer_id;
  char is_ready;
};

struct QueueStatus
{
  int32_t players_count;
  struct PlayerQueueInfo **players_in_queue;
};

/* queue state */
struct ButtonPressed
{
  char all_button_codes[5];
};

struct InitialLocations
{
  int32_t map_width;
  int32_t map_height;
  char *tiles;
};

struct PlayerGameInfo
{
  char username[32];
  int32_t gamer_id;
  float x;
  float y;
  float score;
};

struct ObjectInfo
{
  int32_t id;
  char type;
  float x;
  float y;
};

struct GameState
{
  int32_t players_count;
  int32_t objects_count;
  struct PlayerGameInfo **players;
  struct ObjectInfo **objects;
};

struct Notification
{
  char message[256];
};

/* after-game state */
struct PlayerScoreboardInfo
{
  char username[32];
  int32_t gamer_id;
  float score;
  int32_t stats_count;
  /* max stat_info line is 128 */
  char *stats_info;
};

struct Scoreboard
{
  int32_t players_count;
  struct PlayerScoreboardInfo **players;
};
#pragma pack(pop)

#endif