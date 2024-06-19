#ifndef PACKETS_SERVER_H
#define PACKETS_SERVER_H

#include "packets_common.h"
#include <stdint.h>

/* basic functions */
void initDataServer();
void freeDataServer();

void addHeaderServer(uint16_t msg_type, int32_t packet_size, int32_t player_id);
void addFooterServer(char end);

/* server to client send */
void addJoinQueueResponseToData(struct JoinQueueResponse *join_resp, int32_t player_id, int32_t is_last);
void addPlayerQueueInfoToData(struct PlayerQueueInfo *player_queue);
void addQueueStatusToData(struct QueueStatus *queue_status, int32_t player_id, int32_t is_last);
void addInitialLocationsToData(struct InitialLocations *loc, int32_t player_id, int32_t is_last);
void addPlayerGameInfoToData(struct PlayerGameInfo *player_game);
void addGameReadyResponseToData(int32_t player_id, int32_t is_last);
void addObjectInfoToData(struct ObjectInfo *obj);
void addGameStateToData(struct GameState *game_state, int32_t player_id, int32_t is_last);
void addNotificationToData(struct Notification *note, int32_t player_id, int32_t is_last);
void addGameOverToData(int32_t player_id, int32_t is_last);
void addPlayerScoreboardInfoToData(struct PlayerScoreboardInfo *player_board);
void addScoreboardToData(struct Scoreboard *board, int32_t player_id, int32_t is_last);

/* client to server parse */
struct JoinQueueRequest *getJoinQueueRequest(unsigned char *msg);
struct ButtonPressed *getButtonPressed(unsigned char *msg);

/* helper functions for testing */
int getDataBufferServer(unsigned char *res, int32_t packet_size);
int getDataEndServer();

#endif