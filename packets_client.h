#ifndef PACKETS_CLIENT_H
#define PACKETS_CLIENT_H

#include "packets_common.h"
#include <stdint.h>

/* basic functions */
void initDataClient();
void freeDataClient();

void addHeaderClient(uint16_t msg_type, int32_t packet_size, int32_t player_id);
void addFooterClient(char end);

/* client to server send */
void addJoinQueueRequest(struct JoinQueueRequest *q_req, int32_t player_id, int32_t is_last);
void addButtonPressed(struct ButtonPressed *btn_pressed, int32_t player_id, int32_t is_last);
void addLeaveQueueRequest(int32_t player_id, int32_t is_last);
void addPlayerReadyRequest(int32_t player_id, int32_t is_last);

/* server to client parse */
struct JoinQueueResponse *getJoinQueueResponse(unsigned char *msg);
struct QueueStatus *getQueueStatus(unsigned char *msg);
struct InitialLocations *getInitialLocations(unsigned char *msg);
struct GameState *getGameState(unsigned char *msg);
struct Notification *getNotification(unsigned char *msg);
struct Scoreboard *getScoreboard(unsigned char *msg);

/* helper functions for testing */
int getDataBufferClient(unsigned char *res, int32_t packet_size);
int getDataEndClient();

#endif