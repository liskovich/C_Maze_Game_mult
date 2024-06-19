#ifndef HELPERS_COMMON_H
#define HELPERS_COMMON_H

#include "packets_common.h"
#include <stdint.h>

/* testing helpers */
void print_buffer_hex(unsigned char *buffer, long length);

/* general packet data */
char getEndSymbol(int32_t is_last);
int32_t calcCRC(unsigned char *data, int32_t size);

struct Header *getHeader(unsigned char *msg);
struct Footer *getFooter(unsigned char *msg);

int32_t getHeaderSize();
int32_t getFooterSize();

/* packet-specific functions */
int32_t getJoinQueueResponseSize();
int32_t getPlayerQueueInfoSize();
int32_t getQueueStatusSize(struct QueueStatus *queue_status);
int32_t getInitialLocationsSize(struct InitialLocations *loc);
int32_t getPlayerGameInfoSize();
int32_t getObjectInfoSize();
int32_t getGameStateSize(struct GameState *game_state);
int32_t getNotificationSize();
int32_t getPlayerScoreboardInfoSize(struct PlayerScoreboardInfo *player_board);
int32_t getScoreboardSize(struct Scoreboard *board);

int32_t getJoinQueueRequestSize();
int32_t getButtonPressedSize();

#endif