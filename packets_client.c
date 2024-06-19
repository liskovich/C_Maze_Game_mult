#include "helpers_common.h"
#include "packets_common.h"
#include "packets_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

#define MAX_MSG_BUFF_SIZE 1024
#define MAX_PACKET_SIZE 1024

#define START 99
#define END 77

/* Buffer where the new message is created before sending */
unsigned char *dataBufferClient;
/* Index of the next free space in buffer, remember to update when adding stuff */
int dataEndClient;

/* Message id, increase after sending each message */
int32_t message_id_client = 0;
/* Packet order, increase for each added packet in message */
/* Back to 0 after sending the message */
int32_t packet_order_client = 0;

void initDataClient()
{
  dataBufferClient = (unsigned char *)malloc(MAX_MSG_BUFF_SIZE * MAX_PACKET_SIZE * sizeof(unsigned char));
  if (dataBufferClient == NULL)
  {
    printf("Failed to allocate msg buffer\n");
  }
  printf("Allocated data buffer successfully\n");
  dataEndClient = 0;
}

void freeDataClient()
{
  if (dataBufferClient != NULL)
  {
    free(dataBufferClient);
    dataBufferClient = NULL;
    dataEndClient = 0;
    printf("Freed data buffer successfully\n");
  }
  else
  {
    printf("Failed to free msg buffer\n");
  }
}

void addJoinQueueRequest(struct JoinQueueRequest *q_req, int32_t player_id, int32_t is_last)
{
  /* add header */
  int32_t packet_size = getJoinQueueRequestSize();
  printf("Packet size for JoinQueueRequest: %d\n", packet_size);

  addHeaderClient(JOIN_QUEUE_REQ, packet_size, player_id);

  /* add username */
  memcpy(dataBufferClient + dataEndClient, &(q_req->username), 32 * sizeof(char));
  dataEndClient += 32 * sizeof(char);

  /* add footer */
  addFooterClient(getEndSymbol(is_last));
}

void addButtonPressed(struct ButtonPressed *btn_pressed, int32_t player_id, int32_t is_last)
{
  /* add header */
  int32_t packet_size = getButtonPressedSize();
  printf("Packet size for ButtonPressed: %d\n", packet_size);

  addHeaderClient(BTN_PRESSED, packet_size, player_id);

  /* add all_button_codes */
  memcpy(dataBufferClient + dataEndClient, &(btn_pressed->all_button_codes), 5 * sizeof(char));
  dataEndClient += 5 * sizeof(char);

  /* add footer */
  addFooterClient(getEndSymbol(is_last));
}

void addLeaveQueueRequest(int32_t player_id, int32_t is_last)
{
  /* add header */
  int32_t packet_size = 0;
  addHeaderClient(LEAVE_QUEUE_REQ, packet_size, player_id);

  /* add footer */
  addFooterClient(getEndSymbol(is_last));
}

void addPlayerReadyRequest(int32_t player_id, int32_t is_last)
{
  /* add header */
  int32_t packet_size = 0;
  addHeaderClient(PLAYER_READY_REQ, packet_size, player_id);

  /* add footer */
  addFooterClient(getEndSymbol(is_last));
}

void addHeaderClient(uint16_t msg_type, int32_t packet_size, int32_t player_id)
{
  /*
  TODO: fix endianess
  */

  /* add SOH */
  char soh = 1;
  memcpy(dataBufferClient + dataEndClient, &soh, sizeof(char));
  dataEndClient += sizeof(char);

  /* add msg id */
  memcpy(dataBufferClient + dataEndClient, &message_id_client, sizeof(int32_t));
  dataEndClient += sizeof(int32_t);

  /* add msg type */
  memcpy(dataBufferClient + dataEndClient, &msg_type, sizeof(uint16_t));
  dataEndClient += sizeof(uint16_t);

  /* add packet size */
  memcpy(dataBufferClient + dataEndClient, &packet_size, sizeof(int32_t));
  dataEndClient += sizeof(int32_t);

  /* add packet order */
  memcpy(dataBufferClient + dataEndClient, &packet_order_client, sizeof(int32_t));
  dataEndClient += sizeof(int32_t);

  /* add player id */
  memcpy(dataBufferClient + dataEndClient, &player_id, sizeof(int32_t));
  dataEndClient += sizeof(int32_t);

  /* add STX */
  char stx = 2;
  memcpy(dataBufferClient + dataEndClient, &stx, sizeof(char));
  dataEndClient += sizeof(char);

  /* Fill data size correctly in header, accounting for CRC and END symbol */
  printf("Added packet header successfully!\n");
}

void addFooterClient(char end)
{
  /*
  TODO: fix endianess
  */

  /* add checksum */
  /*
  TODO: pass correct data to calculate and store CRC
  */
  int32_t crc = calcCRC(dataBufferClient, 0);
  memcpy(dataBufferClient + dataEndClient, &crc, sizeof(int32_t));
  dataEndClient += sizeof(int32_t);

  /* add END symbol */
  memcpy(dataBufferClient + dataEndClient, &end, sizeof(char));
  dataEndClient += sizeof(char);

  printf("Added packet footer successfully!\n");
}

struct JoinQueueResponse *getJoinQueueResponse(unsigned char *msg)
{
  struct JoinQueueResponse *response = malloc(sizeof(struct JoinQueueResponse));
  if (response == NULL)
  {
    return NULL;
  }

  /* get success */
  memcpy(&(response->success), msg, sizeof(char));

  return response;
}

struct QueueStatus *getQueueStatus(unsigned char *msg)
{
  struct QueueStatus *queue_status = malloc(sizeof(struct QueueStatus));
  if (queue_status == NULL)
  {
    return NULL;
  }

  /* get players_count */
  memcpy(&(queue_status->players_count), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* memory for the players_in_queue array */
  queue_status->players_in_queue = malloc(queue_status->players_count * sizeof(struct PlayerQueueInfo *));
  if (queue_status->players_in_queue == NULL)
  {
    free(queue_status);
    return NULL;
  }

  /* get PlayerQueueInfo for each player */
  int i;
  for (i = 0; i < queue_status->players_count; i++)
  {
    /* memory for PlayerQueueInfo object */
    queue_status->players_in_queue[i] = malloc(sizeof(struct PlayerQueueInfo));
    if (queue_status->players_in_queue[i] == NULL)
    {
      int j;
      for (j = 0; j < i; j++)
      {
        free(queue_status->players_in_queue[j]);
      }
      free(queue_status->players_in_queue);
      free(queue_status);
      return NULL;
    }

    /* get username */
    memcpy(queue_status->players_in_queue[i]->username, msg, 32 * sizeof(char));
    msg += 32 * sizeof(char);

    /* get gamer_id */
    memcpy(&(queue_status->players_in_queue[i]->gamer_id), msg, sizeof(int32_t));
    msg += sizeof(int32_t);

    /* get is_ready */
    memcpy(&(queue_status->players_in_queue[i]->is_ready), msg, sizeof(char));
    msg += sizeof(char);
  }

  return queue_status;
}

struct InitialLocations *getInitialLocations(unsigned char *msg)
{
  struct InitialLocations *initial_locations = malloc(sizeof(struct InitialLocations));
  if (initial_locations == NULL)
  {
    return NULL;
  }

  /* get map_width */
  memcpy(&(initial_locations->map_width), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get map_height */
  memcpy(&(initial_locations->map_height), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* memory for tiles */
  int tiles_size = initial_locations->map_width * initial_locations->map_height * sizeof(char);
  initial_locations->tiles = malloc(tiles_size);
  if (initial_locations->tiles == NULL)
  {
    free(initial_locations);
    return NULL;
  }

  /* get tiles */
  memcpy(initial_locations->tiles, msg, tiles_size);

  return initial_locations;
}

struct GameState *getGameState(unsigned char *msg)
{
  struct GameState *game_state = malloc(sizeof(struct GameState));
  if (game_state == NULL)
  {
    return NULL;
  }

  /* get players_count */
  memcpy(&(game_state->players_count), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get objects_count */
  memcpy(&(game_state->objects_count), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* memory for the players array */
  game_state->players = malloc(game_state->players_count * sizeof(struct PlayerGameInfo *));
  if (game_state->players == NULL)
  {
    free(game_state);
    return NULL;
  }

  /* get PlayerGameInfo for each player */
  int i;
  for (i = 0; i < game_state->players_count; i++)
  {
    /* memory for PlayerGameInfo object */
    game_state->players[i] = malloc(sizeof(struct PlayerGameInfo));
    if (game_state->players[i] == NULL)
    {
      int j;
      for (j = 0; j < i; j++)
      {
        free(game_state->players[j]);
      }
      free(game_state->players);
      free(game_state);
      return NULL;
    }

    /* get username */
    memcpy(game_state->players[i]->username, msg, 32 * sizeof(char));
    msg += 32 * sizeof(char);

    /* get gamer_id */
    memcpy(&(game_state->players[i]->gamer_id), msg, sizeof(int32_t));
    msg += sizeof(int32_t);

    /* get x */
    memcpy(&(game_state->players[i]->x), msg, sizeof(float));
    msg += sizeof(float);

    /* get y */
    memcpy(&(game_state->players[i]->y), msg, sizeof(float));
    msg += sizeof(float);

    /* get score */
    memcpy(&(game_state->players[i]->score), msg, sizeof(float));
    msg += sizeof(float);
  }

  /* memory for the objects array */
  game_state->objects = malloc(game_state->objects_count * sizeof(struct ObjectInfo *));
  if (game_state->objects == NULL)
  {
    int i;
    for (i = 0; i < game_state->players_count; i++)
    {
      free(game_state->players[i]);
    }
    free(game_state->players);
    free(game_state);
    return NULL;
  }

  /* get ObjectInfo for each object */
  for (i = 0; i < game_state->objects_count; i++)
  {
    /* memory for ObjectInfo object */
    game_state->objects[i] = malloc(sizeof(struct ObjectInfo));
    if (game_state->objects[i] == NULL)
    {
      int j;
      for (j = 0; j < i; j++)
      {
        free(game_state->objects[j]);
      }
      free(game_state->objects);
      for (j = 0; j < game_state->players_count; j++)
      {
        free(game_state->players[j]);
      }
      free(game_state->players);
      free(game_state);
      return NULL;
    }

    /* get id */
    memcpy(&(game_state->objects[i]->id), msg, sizeof(int32_t));
    msg += sizeof(int32_t);

    /* get type */
    memcpy(&(game_state->objects[i]->type), msg, sizeof(char));
    msg += sizeof(char);

    /* get x */
    memcpy(&(game_state->objects[i]->x), msg, sizeof(float));
    msg += sizeof(float);

    /* get y */
    memcpy(&(game_state->objects[i]->y), msg, sizeof(float));
    msg += sizeof(float);
  }

  return game_state;
}

struct Notification *getNotification(unsigned char *msg)
{
  struct Notification *notification = malloc(sizeof(struct Notification));
  if (notification == NULL)
  {
    return NULL;
  }

  /* get message */
  memcpy(&(notification->message), msg, 256 * sizeof(char));

  return notification;
}

struct Scoreboard *getScoreboard(unsigned char *msg)
{
  struct Scoreboard *scoreboard = malloc(sizeof(struct Scoreboard));
  if (scoreboard == NULL)
  {
    return NULL;
  }

  /* get players_count */
  memcpy(&(scoreboard->players_count), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* memory for the players array */
  scoreboard->players = malloc(scoreboard->players_count * sizeof(struct PlayerScoreboardInfo *));
  if (scoreboard->players == NULL)
  {
    free(scoreboard);
    return NULL;
  }

  /* get PlayerScoreboardInfo for each player */
  int i;
  for (i = 0; i < scoreboard->players_count; i++)
  {
    /* memory for PlayerScoreboardInfo object */
    scoreboard->players[i] = malloc(sizeof(struct PlayerScoreboardInfo));
    if (scoreboard->players[i] == NULL)
    {
      int j;
      for (j = 0; j < i; j++)
      {
        free(scoreboard->players[j]->stats_info);
        free(scoreboard->players[j]);
      }
      free(scoreboard->players);
      free(scoreboard);
      return NULL;
    }

    /* get username */
    memcpy(scoreboard->players[i]->username, msg, sizeof(char) * 32);
    msg += sizeof(char) * 32;

    /* get gamer_id */
    memcpy(&(scoreboard->players[i]->gamer_id), msg, sizeof(int32_t));
    msg += sizeof(int32_t);

    /* get score */
    memcpy(&(scoreboard->players[i]->score), msg, sizeof(float));
    msg += sizeof(float);

    /* get stats_count */
    memcpy(&(scoreboard->players[i]->stats_count), msg, sizeof(int32_t));
    msg += sizeof(int32_t);

    /* memory for stats_info */
    scoreboard->players[i]->stats_info = malloc(scoreboard->players[i]->stats_count * sizeof(char) * 128);
    if (scoreboard->players[i]->stats_info == NULL)
    {
      int j;
      for (j = 0; j <= i; j++)
      {
        free(scoreboard->players[j]->stats_info);
        free(scoreboard->players[j]);
      }
      free(scoreboard->players);
      free(scoreboard);
      return NULL;
    }

    /* get stats_info */
    memcpy(scoreboard->players[i]->stats_info, msg, scoreboard->players[i]->stats_count * sizeof(char) * 128);
    msg += scoreboard->players[i]->stats_count * sizeof(char) * 128;
  }

  return scoreboard;
}

/* helper functions to test created packets */
/* something similar might be used on client, to read server-sent buffer */

/*
TODO: modify to get correct packet length
*/
int getDataBufferClient(unsigned char *res, int32_t packet_size)
{
  /*
  int full_len = packet_size;

  int i;
  for (i = 0; i < full_len; i++)
  {
    res[i] = dataBufferClient[i];
  }
  return full_len;
  */
  memcpy(res, dataBufferClient, packet_size * sizeof(unsigned char));
  return packet_size;
}

int getDataEndClient()
{
  return dataEndClient;
}