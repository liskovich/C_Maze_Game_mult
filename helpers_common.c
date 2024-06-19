#include "helpers_common.h"
#include "packets_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
testing helpers
*/

void print_buffer_hex(unsigned char *buffer, long length)
{
  long i;
  for (i = 0; i < length; i++)
  {
    printf("%02X ", buffer[i]);
  }
  printf("\n");
}

/*
general packet data
*/

/* to determine if there will be more packets in the message or not */
char getEndSymbol(int32_t is_last)
{
  char end;
  if (is_last == 0)
  {
    end = 23;
  }
  else
  {
    end = 4;
  }
  return end;
}

int32_t calcCRC(unsigned char *data, int32_t size)
{
  int32_t sum = 123;

  /*
  TODO: calculate CRC
  */
  (void)data;
  (void)size;

  return sum;
}

struct Header *getHeader(unsigned char *msg)
{
  /* Get header struct and check for start symbol */
  struct Header *h = malloc(sizeof(struct Header));
  if (h == NULL)
  {
    return NULL;
  }

  /* get SOH */
  memcpy(&(h->soh), msg, sizeof(char));
  msg += sizeof(char);

  /* get msg_id */
  memcpy(&(h->msg_id), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get msg_type */
  memcpy(&(h->msg_type), msg, sizeof(uint16_t));
  msg += sizeof(uint16_t);

  /* get size */
  memcpy(&(h->size), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get order */
  memcpy(&(h->order), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get player_id */
  memcpy(&(h->player_id), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get STX */
  memcpy(&(h->stx), msg, sizeof(char));
  msg += sizeof(char);

  return h;
}

struct Footer *getFooter(unsigned char *msg)
{
  /* Get footer struct and check for CRC and END symbol */
  struct Footer *f = malloc(sizeof(struct Footer));
  if (f == NULL)
  {
    return NULL;
  }

  /* get checksum */
  memcpy(&(f->checksum), msg, sizeof(int32_t));
  msg += sizeof(int32_t);

  /* get END */
  memcpy(&(f->end), msg, sizeof(char));
  msg += sizeof(char);

  return f;
}

int32_t getHeaderSize()
{
  return sizeof(struct Header);
}

int32_t getFooterSize()
{
  return sizeof(struct Footer);
}

/*
packet-specific functions
*/

int32_t getJoinQueueResponseSize()
{
  int32_t total_size = 0;
  total_size += sizeof(char);
  return total_size;
}

int32_t getPlayerQueueInfoSize()
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);
  total_size += sizeof(char);
  total_size += 32 * sizeof(char);
  return total_size;
}

int32_t getQueueStatusSize(struct QueueStatus *queue_status)
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);
  total_size += queue_status->players_count * getPlayerQueueInfoSize();
  return total_size;
}

int32_t getInitialLocationsSize(struct InitialLocations *loc)
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);
  total_size += sizeof(int32_t);
  /* store 2D map as 1D array */
  /* append each next row at the end of previous */
  total_size += loc->map_width * loc->map_height * sizeof(char);
  return total_size;
}

int32_t getPlayerGameInfoSize()
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);
  total_size += 32 * sizeof(char);
  total_size += sizeof(float);
  total_size += sizeof(float);
  total_size += sizeof(float);
  return total_size;
}

int32_t getObjectInfoSize()
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);
  total_size += sizeof(char);
  total_size += sizeof(float);
  total_size += sizeof(float);
  return total_size;
}

int32_t getGameStateSize(struct GameState *game_state)
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);
  total_size += sizeof(int32_t);
  total_size += game_state->players_count * getPlayerGameInfoSize();
  total_size += game_state->objects_count * getObjectInfoSize();
  return total_size;
}

int32_t getNotificationSize()
{
  int32_t total_size = 0;
  total_size += 256 * sizeof(char);
  return total_size;
}

int32_t getPlayerScoreboardInfoSize(struct PlayerScoreboardInfo *player_board)
{
  int32_t total_size = 0;
  total_size += 32 * sizeof(char);
  total_size += sizeof(int32_t);
  total_size += sizeof(int32_t);
  total_size += sizeof(float);
  total_size += player_board->stats_count * 128 * sizeof(char);
  return total_size;
}

int32_t getScoreboardSize(struct Scoreboard *board)
{
  int32_t total_size = 0;
  total_size += sizeof(int32_t);

  if (&board->players != NULL)
  {
    int i;
    for (i = 0; i < board->players_count; i++)
    {
      total_size += getPlayerScoreboardInfoSize(board->players[i]);
    }
  }
  return total_size;
}

int32_t getJoinQueueRequestSize()
{
  int32_t total_size = 0;
  total_size += 32 * sizeof(char);
  return total_size;
}

int32_t getButtonPressedSize()
{
  int32_t total_size = 0;
  total_size += 5 * sizeof(char);
  return total_size;
}