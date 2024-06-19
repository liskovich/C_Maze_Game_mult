#include "helpers_common.h"
#include "packets_common.h"
#include "packets_server.h"
#include "packets_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int writePacketToFile(unsigned char *temp, int buffSize, char *filename)
{
  char formatted_filename[256];
  sprintf(formatted_filename, "%s_output.bin", filename);

  int fd;
  fd = open(formatted_filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1)
  {
    perror("open");
    return 1;
  }

  ssize_t written = write(fd, temp, buffSize);
  if (written == -1)
  {
    perror("write");
    close(fd);
    return 1;
  }
  else if (written != buffSize)
  {
    printf("Warning: Not all bytes written!\n");
  }
  close(fd);
  return 0;
}

/* 
functions to simulate received packets from the server 
*/

int testJoinQueueResponsePacketReceive()
{
  printf("Testing JoinQueueResponse packet...\n");

  /* create sample test packet */
  struct JoinQueueResponse *q_resp;
  q_resp = malloc(sizeof(struct JoinQueueResponse));
  q_resp->success = 1;

  printf("Created test objects\n");
  initDataServer();

  /* for now, work in small endian mode */
  addJoinQueueResponseToData(q_resp, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getJoinQueueResponseSize() + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "join_queue_resp");

  freeDataServer();
  free(q_resp);
  printf("JoinQueueResponse packet written to file!\n");
  return 0;
}

int testQueueStatusPacketReceive()
{
  printf("Testing QueueStatus packet...\n");

  /* create sample test packet */
  struct PlayerQueueInfo player1;
  struct PlayerQueueInfo player2;

  strcpy(player1.username, "ricard");
  player1.gamer_id = 1;
  player1.is_ready = 1;

  strcpy(player2.username, "timur");
  player2.gamer_id = 2;
  player2.is_ready = 1;

  struct QueueStatus *q_status = (struct QueueStatus *)malloc(sizeof(struct QueueStatus));
  q_status->players_count = 2;

  q_status->players_in_queue = (struct PlayerQueueInfo **)malloc(q_status->players_count * sizeof(struct PlayerQueueInfo *));
  q_status->players_in_queue[0] = &player1;
  q_status->players_in_queue[1] = &player2;

  printf("Created test objects\n");
  initDataServer();

  /* for now, work in small endian mode */
  addQueueStatusToData(q_status, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getQueueStatusSize(q_status) + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "queue_status");

  freeDataServer();
  free(q_status->players_in_queue);
  free(q_status);
  printf("QueueStatus packet written to file!\n");
  return 0;
}

int testInitialLocationsReceive()
{
  printf("Testing InitialLocations packet...\n");

  /* create sample test packet */
  struct InitialLocations *initial_locations = (struct InitialLocations *)malloc(sizeof(struct InitialLocations));
  initial_locations->map_width = 15;
  initial_locations->map_height = 15;

  int mem_size = initial_locations->map_width * initial_locations->map_height * sizeof(char);
  initial_locations->tiles = (char *)malloc(mem_size);

  /* fill tiles with '1' for demo purposes */
  int i;
  for (i = 0; i < initial_locations->map_width * initial_locations->map_height; i++)
  {
    initial_locations->tiles[i] = 1;
  }

  printf("Created test objects\n");
  initDataServer();

  /* for now, work in small endian mode */
  addInitialLocationsToData(initial_locations, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getInitialLocationsSize(initial_locations) + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "initial_locations");

  freeDataServer();
  free(initial_locations->tiles);
  free(initial_locations);
  printf("InitialLocations packet written to file!\n");
  return 0;
}

int testGameReadyResponsePacketReceive()
{
  printf("Testing GameReadyResponse packet...\n");

  /* create sample test packet */
  initDataServer();

  /* for now, work in small endian mode */
  addGameReadyResponseToData(3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "game_ready_resp");

  freeDataServer();
  printf("GameReadyResponse packet written to file!\n");
  return 0;
}

int testGameStatePacketReceive()
{
  printf("Testing GameState packet...\n");

  /* create sample test packet */
  struct PlayerGameInfo player1;
  struct PlayerGameInfo player2;

  strcpy(player1.username, "ricard");
  player1.gamer_id = 1;
  player1.score = 15;
  player1.x = 5;
  player1.y = 6;

  strcpy(player2.username, "timur");
  player2.gamer_id = 2;
  player2.score = 14;
  player2.x = 7;
  player2.y = 8;

  struct ObjectInfo obj1;
  struct ObjectInfo obj2;
  struct ObjectInfo obj3;

  obj1.id = 18;
  obj1.type = 'c';
  obj1.x = 13;
  obj1.y = 13;

  obj2.id = 19;
  obj2.type = 'w';
  obj2.x = 11;
  obj2.y = 10;

  obj3.id = 20;
  obj3.type = 's';
  obj3.x = 9;
  obj3.y = 12;

  struct GameState *game_state = (struct GameState *)malloc(sizeof(struct GameState));
  game_state->players_count = 2;
  game_state->objects_count = 3;

  game_state->players = (struct PlayerGameInfo **)malloc(game_state->players_count * sizeof(struct PlayerGameInfo *));
  game_state->players[0] = &player1;
  game_state->players[1] = &player2;

  game_state->objects = (struct ObjectInfo **)malloc(game_state->objects_count * sizeof(struct ObjectInfo *));
  game_state->objects[0] = &obj1;
  game_state->objects[1] = &obj2;
  game_state->objects[2] = &obj3;

  printf("Created test objects\n");
  initDataServer();

  /* for now, work in small endian mode */
  addGameStateToData(game_state, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getGameStateSize(game_state) + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "game_state");

  freeDataServer();
  free(game_state->objects);
  free(game_state->players);
  free(game_state);
  printf("GameState packet written to file!\n");
  return 0;
}

int testNotificationPacketReceive()
{
  printf("Testing Notification packet...\n");

  /* create sample test packet */
  struct Notification *note;
  note = malloc(sizeof(struct Notification));
  strcpy(note->message, "what's up?");

  printf("Created test objects\n");
  initDataServer();

  /* for now, work in small endian mode */
  addNotificationToData(note, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getNotificationSize() + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "notification");

  freeDataServer();
  free(note);
  printf("Notification packet written to file!\n");
  return 0;
}

int testGameOverPacketReceive()
{
  printf("Testing GameOver packet...\n");

  /* create sample test packet */
  initDataServer();

  /* for now, work in small endian mode */
  addGameOverToData(3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "game_over");

  freeDataServer();
  printf("GameOver packet written to file!\n");
  return 0;
}

int testScoreboardPacketReceive()
{
  printf("Testing Scoreboard packet...\n");

  /* create sample test packet */
  struct PlayerScoreboardInfo player1;
  struct PlayerScoreboardInfo player2;

  strcpy(player1.username, "ricard");
  player1.gamer_id = 1;
  player1.score = 15;
  player1.stats_count = 2;
  player1.stats_info = malloc(player1.stats_count * 128 * sizeof(char));

  char *p1_stat_strings[] = {"health: 80/100", "food: 100/100"};
  int i;
  for (i = 0; i < player1.stats_count; i++)
  {
    int start_index = i * 128;
    strncpy(player1.stats_info + start_index, p1_stat_strings[i], 128);
  }

  printf("Player 1 stats list:\n%s\n", player1.stats_info);

  strcpy(player2.username, "timur");
  player2.gamer_id = 2;
  player2.score = 14;
  player2.stats_count = 2;
  player2.stats_info = malloc(player2.stats_count * 128 * sizeof(char));

  char *p2_stat_strings[] = {"health: 90/100", "food: 70/100"};
  for (i = 0; i < player2.stats_count; i++)
  {
    int start_index = i * 128;
    strncpy(player2.stats_info + start_index, p2_stat_strings[i], 128);
  }

  printf("Player 2 stats list:\n%s\n", player2.stats_info);

  struct Scoreboard *board = (struct Scoreboard *)malloc(sizeof(struct Scoreboard));
  board->players_count = 2;

  board->players = (struct PlayerScoreboardInfo **)malloc(board->players_count * sizeof(struct PlayerScoreboardInfo *));
  board->players[0] = &player1;
  board->players[1] = &player2;

  printf("Created test objects\n");
  initDataServer();

  /* for now, work in small endian mode */
  addScoreboardToData(board, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getScoreboardSize(board) + getFooterSize();
  int buffSize = getDataBufferServer(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndServer();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "scoreboard");

  freeDataServer();
  free(board->players[0]->stats_info);
  free(board->players[1]->stats_info);
  free(board->players);
  free(board);
  printf("Scoreboard packet written to file!\n");
  return 0;
}

/* 
functions to simulate received packets from the client 
*/

int testJoinQueueRequestPacketReceive()
{
  printf("Testing JoinQueueRequest packet...\n");

  /* create sample test packet */
  struct JoinQueueRequest *q_req;
  q_req = malloc(sizeof(struct JoinQueueRequest));
  strcpy(q_req->username, "ricard");

  printf("Created test objects\n");
  initDataClient();

  /* for now, work in small endian mode */
  addJoinQueueRequest(q_req, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getJoinQueueRequestSize() + getFooterSize();
  int buffSize = getDataBufferClient(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndClient();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "join_queue_req");

  freeDataClient();
  free(q_req);
  printf("JoinQueueRequest packet written to file!\n");
  return 0;
}

int testButtonPressedPacketReceive()
{
  printf("Testing ButtonPressed packet...\n");

  /* create sample test packet */
  struct ButtonPressed *btn_pressed;
  btn_pressed = malloc(sizeof(struct ButtonPressed));
  strcpy(btn_pressed->all_button_codes, "38");

  printf("Created test objects\n");
  initDataClient();

  /* for now, work in small endian mode */
  addButtonPressed(btn_pressed, 3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getButtonPressedSize() + getFooterSize();
  int buffSize = getDataBufferClient(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndClient();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "btn_pressed");

  freeDataClient();
  free(btn_pressed);
  printf("ButtonPressed packet written to file!\n");
  return 0;
}

int testLeaveQueueRequestPacketReceive()
{
  printf("Testing LeaveQueueRequest packet...\n");

  /* create sample test packet */
  initDataClient();

  /* for now, work in small endian mode */
  addLeaveQueueRequest(3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getFooterSize();
  int buffSize = getDataBufferClient(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndClient();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "leave_queue_req");

  freeDataClient();
  printf("LeaveQueueRequest packet written to file!\n");
  return 0;
}

int testPlayerReadyRequestPacketReceive()
{
  printf("Testing PlayerReadyRequest packet...\n");

  /* create sample test packet */
  initDataClient();

  /* for now, work in small endian mode */
  addPlayerReadyRequest(3, 1);

  unsigned char res[1024 * 1024];
  int packet_size = getHeaderSize() + getFooterSize();
  int buffSize = getDataBufferClient(res, packet_size);

  /* move data to output buffer for easier work */
  unsigned char temp[buffSize];
  int i;
  for (i = 0; i < buffSize; i++)
  {
    temp[i] = res[i];
  }

  /* check buffer size and data cursor position */
  printf("Size of data to write: %d\n", buffSize);

  int dataEndCursor = getDataEndClient();
  printf("Data end cursor: %d\n", dataEndCursor);

  /* write packet content to file */
  writePacketToFile(temp, buffSize, "player_ready_req");

  freeDataClient();
  printf("PlayerReadyRequest packet written to file!\n");
  return 0;
}

int main()
{
  /* uncomment to test */

  /* testJoinQueueResponsePacketReceive(); */
  /* testQueueStatusPacketReceive(); */
  /* testInitialLocationsReceive(); */
  /* testGameReadyResponsePacketReceive(); */
  /* testGameStatePacketReceive(); */
  /* testNotificationPacketReceive(); */
  /* testGameOverPacketReceive(); */
  /* testScoreboardPacketReceive(); */

  /* testJoinQueueRequestPacketReceive(); */
  /* testButtonPressedPacketReceive(); */
  /* testLeaveQueueRequestPacketReceive(); */
  /* testPlayerReadyRequestPacketReceive(); */
  return 0;
}