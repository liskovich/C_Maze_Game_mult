#include "helpers_common.h"
#include "packets_common.h"
#include "packets_server.h"

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
unsigned char *dataBufferServer;
/* Index of the next free space in buffer, remember to update when adding stuff */
int dataEndServer;

/* Message id, increase after sending each message */
int32_t message_id_server = 0;
/* Packet order, increase for each added packet in message */
/* Back to 0 after sending the message */
int32_t packet_order_server = 0;

void initDataServer()
{
  dataBufferServer = (unsigned char *)malloc(MAX_MSG_BUFF_SIZE * MAX_PACKET_SIZE * sizeof(unsigned char));
  if (dataBufferServer == NULL)
  {
    printf("Failed to allocate msg buffer\n");
  }
  printf("Allocated data buffer successfully\n");
  dataEndServer = 0;
}

void freeDataServer()
{
  if (dataBufferServer != NULL)
  {
    free(dataBufferServer);
    dataBufferServer = NULL;
    dataEndServer = 0;
    printf("Freed data buffer successfully\n");
  }
  else
  {
    printf("Failed to free msg buffer\n");
  }
}

void addJoinQueueResponseToData(struct JoinQueueResponse *join_resp, int32_t player_id, int32_t is_last)
{
  /* add header */
  int32_t packet_size = getJoinQueueResponseSize();
  printf("Packet size for JoinQueueResponse: %d\n", packet_size);

  addHeaderServer(JOIN_QUEUE_RESP, packet_size, player_id);

  /* add reponse status */
  memcpy(dataBufferServer + dataEndServer, &(join_resp->success), sizeof(char));
  dataEndServer += sizeof(char);

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addPlayerQueueInfoToData(struct PlayerQueueInfo *player_queue)
{
  /*
  TODO: fix endianess
  */

  /* add username */
  memcpy(dataBufferServer + dataEndServer, &(player_queue->username), 32 * sizeof(char));
  dataEndServer += 32 * sizeof(char);

  /* add gamer_id */
  memcpy(dataBufferServer + dataEndServer, &(player_queue->gamer_id), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add is ready */
  memcpy(dataBufferServer + dataEndServer, &(player_queue->is_ready), sizeof(char));
  dataEndServer += sizeof(char);
}

void addQueueStatusToData(struct QueueStatus *queue_status, int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = getQueueStatusSize(queue_status);
  printf("Packet size for QueueStatus: %d\n", packet_size);

  addHeaderServer(QUEUE_STATUS, packet_size, player_id);

  /* add player count */
  memcpy(dataBufferServer + dataEndServer, &(queue_status->players_count), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add all players */
  /*
  TODO: check that players not NULL
  */
  int i;
  for (i = 0; i < queue_status->players_count; i++)
  {
    if (queue_status->players_in_queue[i] != NULL)
    {
      addPlayerQueueInfoToData(queue_status->players_in_queue[i]);
    }
    else
    {
      /* create an empty PlayerQueueInfo object */
      struct PlayerQueueInfo *player_empty = (struct PlayerQueueInfo *)malloc(sizeof(struct PlayerQueueInfo));
      char empty_string[32] = "";
      memcpy(player_empty->username, empty_string, 32 * sizeof(char));
      player_empty->gamer_id = -1;
      player_empty->is_ready = 0;

      addPlayerQueueInfoToData(player_empty);
      free(player_empty);
    }
  }

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addInitialLocationsToData(struct InitialLocations *loc, int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = getInitialLocationsSize(loc);
  printf("Packet size for InitialLocations: %d\n", packet_size);

  addHeaderServer(INIT_LOCATIONS, packet_size, player_id);

  /* add map width */
  memcpy(dataBufferServer + dataEndServer, &(loc->map_width), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add map height */
  memcpy(dataBufferServer + dataEndServer, &(loc->map_height), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add map tiles */
  int32_t tiles_mem_size = loc->map_width * loc->map_height * sizeof(char);
  memcpy(dataBufferServer + dataEndServer, loc->tiles, tiles_mem_size);
  dataEndServer += tiles_mem_size;

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addPlayerGameInfoToData(struct PlayerGameInfo *player_game)
{
  /*
  TODO: fix endianess
  */

  /* add username */
  memcpy(dataBufferServer + dataEndServer, &(player_game->username), 32 * sizeof(char));
  dataEndServer += 32 * sizeof(char);

  /* add gamer_id */
  memcpy(dataBufferServer + dataEndServer, &(player_game->gamer_id), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add x */
  memcpy(dataBufferServer + dataEndServer, &(player_game->x), sizeof(float));
  dataEndServer += sizeof(float);

  /* add y */
  memcpy(dataBufferServer + dataEndServer, &(player_game->y), sizeof(float));
  dataEndServer += sizeof(float);

  /* add score */
  memcpy(dataBufferServer + dataEndServer, &(player_game->score), sizeof(float));
  dataEndServer += sizeof(float);
}

void addGameReadyResponseToData(int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = 0;
  addHeaderServer(GAME_READY_RESP, packet_size, player_id);

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addObjectInfoToData(struct ObjectInfo *obj)
{
  /*
  TODO: fix endianess
  */

  /* add object id */
  memcpy(dataBufferServer + dataEndServer, &(obj->id), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add type */
  memcpy(dataBufferServer + dataEndServer, &(obj->type), sizeof(char));
  dataEndServer += sizeof(char);

  /* add x */
  memcpy(dataBufferServer + dataEndServer, &(obj->x), sizeof(float));
  dataEndServer += sizeof(float);

  /* add y */
  memcpy(dataBufferServer + dataEndServer, &(obj->y), sizeof(float));
  dataEndServer += sizeof(float);
}

void addGameStateToData(struct GameState *game_state, int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = getGameStateSize(game_state);
  addHeaderServer(GAME_STATE, packet_size, player_id);

  /* add player count */
  memcpy(dataBufferServer + dataEndServer, &(game_state->players_count), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add object count */
  memcpy(dataBufferServer + dataEndServer, &(game_state->objects_count), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add all players */
  /*
  TODO: check that players not NULL
  */
  int i;
  for (i = 0; i < game_state->players_count; i++)
  {
    addPlayerGameInfoToData(game_state->players[i]);
  }

  /* add all objects */
  /*
  TODO: check that objects not NULL
  */
  for (i = 0; i < game_state->objects_count; i++)
  {
    addObjectInfoToData(game_state->objects[i]);
  }

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addNotificationToData(struct Notification *note, int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = getNotificationSize();
  addHeaderServer(NOTIFICATION, packet_size, player_id);

  /* add message */
  memcpy(dataBufferServer + dataEndServer, &(note->message), 256 * sizeof(char));
  dataEndServer += 256 * sizeof(char);

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addGameOverToData(int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = 0;
  addHeaderServer(GAME_OVER, packet_size, player_id);

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

void addPlayerScoreboardInfoToData(struct PlayerScoreboardInfo *player_board)
{
  /*
  TODO: fix endianess
  */

  /* add username */
  memcpy(dataBufferServer + dataEndServer, &(player_board->username), 32 * sizeof(char));
  dataEndServer += 32 * sizeof(char);

  /* add gamer id */
  memcpy(dataBufferServer + dataEndServer, &(player_board->gamer_id), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add score */
  memcpy(dataBufferServer + dataEndServer, &(player_board->score), sizeof(float));
  dataEndServer += sizeof(float);

  /* add stats count */
  memcpy(dataBufferServer + dataEndServer, &(player_board->stats_count), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add stats */
  /*
  TODO: check that stats not NULL
  */
  int32_t stats_info_size = player_board->stats_count * 128 * sizeof(char);
  memcpy(dataBufferServer + dataEndServer, player_board->stats_info, stats_info_size);
  dataEndServer += stats_info_size;
}

void addScoreboardToData(struct Scoreboard *board, int32_t player_id, int32_t is_last)
{
  /*
  TODO: fix endianess
  */

  /* add header */
  int32_t packet_size = getScoreboardSize(board);
  addHeaderServer(SCORE_BOARD, packet_size, player_id);

  /* add players count */
  memcpy(dataBufferServer + dataEndServer, &(board->players_count), sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add players */
  /*
  TODO: check that players not NULL
  */
  int i;
  for (i = 0; i < board->players_count; i++)
  {
    addPlayerScoreboardInfoToData(board->players[i]);
  }

  /* add footer */
  addFooterServer(getEndSymbol(is_last));
}

/*
general packet data
*/

void addHeaderServer(uint16_t msg_type, int32_t packet_size, int32_t player_id)
{
  /*
  TODO: fix endianess
  */

  /* add SOH */
  char soh = 1;
  memcpy(dataBufferServer + dataEndServer, &soh, sizeof(char));
  dataEndServer += sizeof(char);

  /* add msg id */
  memcpy(dataBufferServer + dataEndServer, &message_id_server, sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add msg type */
  memcpy(dataBufferServer + dataEndServer, &msg_type, sizeof(uint16_t));
  dataEndServer += sizeof(uint16_t);

  /* add packet size */
  memcpy(dataBufferServer + dataEndServer, &packet_size, sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add packet order */
  memcpy(dataBufferServer + dataEndServer, &packet_order_server, sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add player id */
  memcpy(dataBufferServer + dataEndServer, &player_id, sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add STX */
  char stx = 2;
  memcpy(dataBufferServer + dataEndServer, &stx, sizeof(char));
  dataEndServer += sizeof(char);

  /* Fill data size correctly in header, accounting for CRC and END symbol */
  printf("Added packet header successfully!\n");
}

void addFooterServer(char end)
{
  /*
  TODO: fix endianess
  */

  /* add checksum */
  /*
  TODO: pass correct data to calculate and store CRC
  */
  int32_t crc = calcCRC(dataBufferServer, 0);
  memcpy(dataBufferServer + dataEndServer, &crc, sizeof(int32_t));
  dataEndServer += sizeof(int32_t);

  /* add END symbol */
  memcpy(dataBufferServer + dataEndServer, &end, sizeof(char));
  dataEndServer += sizeof(char);

  printf("Added packet footer successfully!\n");
}

struct JoinQueueRequest *getJoinQueueRequest(unsigned char *msg)
{
  struct JoinQueueRequest *req = malloc(sizeof(struct JoinQueueRequest));
  if (req == NULL)
  {
    return NULL;
  }

  /* get username */
  memcpy(&(req->username), msg, 32 * sizeof(char));

  return req;
}

struct ButtonPressed *getButtonPressed(unsigned char *msg)
{
  struct ButtonPressed *btn_pressed = malloc(sizeof(struct ButtonPressed));
  if (btn_pressed == NULL)
  {
    return NULL;
  }

  /* get all_button_codes */
  memcpy(&(btn_pressed->all_button_codes), msg, 5 * sizeof(char));

  return btn_pressed;
}

/* helper functions to test created packets */
/* something similar might be used on client, to read server-sent buffer */

int getDataBufferServer(unsigned char *res, int32_t packet_size)
{
  memcpy(res, dataBufferServer, packet_size * sizeof(unsigned char));
  return packet_size;
}

int getDataEndServer()
{
  return dataEndServer;
}

/* 1. define data structs: DONE */

/* 2. create a buffer to store packet content: DONE */

/* 3. wrap content in a valid network packet */
/* - add header (any other additional information) to packets: DONE */
/* - calculate checksum: TODO */
/* - provider other data relevant for a packet: DONE */
/* - check / adjust endianess: TODO */
/* - check packet element order: DONE */

/* TODO */
/* 3.1. send packets over network */
/* - prepare message buffer for sending (clean, trim, etc.) */
/* - adjust networking for sending large data buffers */

/* 4. parse packets sent from client */
/* - read header (any other additional information) from packets: DONE */
/* - calculate checksum */
/* - get other data relevant from a packet */
/* - store data in structs */