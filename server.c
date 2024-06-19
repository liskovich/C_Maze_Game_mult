#include "helpers_common.h"
#include "packets_common.h"
#include "packets_server.h"
#include "server_logic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

/* shared memory structure for each client connection */
struct ClientData
{
  int client_id;
  int client_socket;
};

#define MAX_CLIENTS 10
#define SHARED_MEM_SIZE (MAX_CLIENTS * sizeof(struct ClientData))
#define MAX_MESSAGE_SIZE (1024 * 1024)
#define SERVER_START_FAIL_ERR 1

#define MIN_WIN_SCORE 100

char *shared_memory = NULL;
struct ClientData *client_data = NULL;

void *client_handler(void *arg);
void start_network(int port);
void parse_packet_server(unsigned char *buffer, int size, int client_socket);
void test_receive_packet_from_file_server(const char *filename);

void server_cleanup();
void server_sigint_handler(int sig);

int main(int argc, char **argv)
{
  /* parse the port provided in args */
  if (argc < 2)
  {
    printf("Provide port:\n %s -p=<PORT_NUMBER>\n", argv[0]);
    return SERVER_START_FAIL_ERR;
  }
  int server_port = atoi(strstr(argv[1], "=") + 1);

  printf("Server started\n");

  /* create shared memory */
  shared_memory = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shared_memory == MAP_FAILED)
  {
    perror("Failed to create shared memory");
    exit(EXIT_FAILURE);
  }
  client_data = (struct ClientData *)shared_memory;

  /* init client_data array with empty slots */
  int i;
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    client_data[i].client_socket = -1;
  }

  /* add signal handlers */
  atexit(server_cleanup);
  signal(SIGINT, server_sigint_handler);

  start_network(server_port);
  return 0;
}

void start_network(int port)
{
  /* standard server setup process */
  int main_socket;
  struct sockaddr_in server_address;
  int client_socket;
  struct sockaddr_in client_address;
  int client_address_size = sizeof(client_address);

  main_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (main_socket < 0)
  {
    printf("Error opening main server socket\n");
    exit(SERVER_START_FAIL_ERR);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port);

  if (bind(main_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
  {
    printf("Error binding to the main server socket\n");
    exit(SERVER_START_FAIL_ERR);
  }

  if (listen(main_socket, MAX_CLIENTS) < 0)
  {
    printf("Error listening to the main server socket\n");
    exit(SERVER_START_FAIL_ERR);
  }

  /* server logic initializations */
  initGameServerLogic(MIN_WIN_SCORE);
  initQueue(MAX_CLIENTS);
  printf("Server logic initialized.\n");

  while (1)
  {
    client_socket = accept(main_socket, (struct sockaddr *)&client_address, (socklen_t *)&client_address_size);
    if (client_socket < 0)
    {
      printf("Error accepting client connection!\n");
      continue;
    }

    /* create thread to handle client requests */
    pthread_t tid;
    pthread_create(&tid, NULL, client_handler, (void *)&client_socket);
  }
}

void *client_handler(void *arg)
{
  int client_socket = *(int *)arg;
  printf("Client socket (inside handler): %d\n", client_socket);

  /* store client id and socket into shared memory */
  int client_id = -1;

  int i;
  for (i = 0; i < MAX_CLIENTS; i++)
  {
    if (client_data[i].client_socket == -1)
    {
      client_id = i;
      client_data[i].client_socket = client_socket;
      break;
    }
  }

  if (client_id == -1)
  {
    printf("No available slots for new client.\n");
    close(client_socket);
    pthread_exit(NULL);
  }

  printf("Client connected: ID = %d\n", client_id);
  unsigned char buffer[MAX_MESSAGE_SIZE];

  /* receive and parse data sent from client */
  while (1)
  {
    ssize_t bytes_received = recv(client_socket, buffer, MAX_MESSAGE_SIZE, 0);
    if (bytes_received <= 0)
    {
      printf("Client disconnected: ID = %d\n", client_id);
      close(client_socket);
      client_data[client_id].client_socket = -1;
      pthread_exit(NULL);
    }

    /* parsing received message */
    printf("Received data from client: ID = %d:\n", client_id);
    print_buffer_hex(buffer, bytes_received);
    parse_packet_server(buffer, bytes_received, client_socket);

    /* send response back to the client */
    /* send(client_socket, response_buffer, response_size, 0); */
  }
  return NULL;
}

void parse_packet_server(unsigned char *buffer, int size, int client_socket)
{
  int localEnd = 0;
  int packet_data_size = 0;

  /* read packet header */
  struct Header *header = getHeader(buffer);

  if ((long unsigned int)size < sizeof(struct Header))
  {
    printf("Incomplete packet received.\n");
    return;
  }

  if (header->soh != 1)
  {
    printf("SOH: invalid char provided.\n");
  }

  printf("msg_id: %d\n", header->msg_id);
  printf("msg_type: %d\n", header->msg_type);
  printf("size: %d\n", header->size);
  printf("order: %d\n", header->order);
  printf("player_id: %d\n", header->player_id);

  if (header->stx != 2)
  {
    printf("STX: invalid char provided.\n");
  }
  localEnd += getHeaderSize();

  /* based on message type, send different responses */
  switch (header->msg_type)
  {
  case JOIN_QUEUE_REQ:
  {
    struct JoinQueueRequest *req = getJoinQueueRequest(buffer + localEnd);
    printf("\n");
    printf("JoinQueueRequest: \n");

    int i;
    for (i = 0; i < 32; i++)
    {
      printf("%c", req->username[i]);
    }
    printf("\n");

    localEnd += getJoinQueueRequestSize();
    packet_data_size += getJoinQueueRequestSize();

    /*
    TODO: send updated QueueStatus to all players in queue
    */

    /* find the player with socket equal to client_socket */
    int player_id = -1;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (client_data[i].client_socket == client_socket)
      {
        player_id = i;
        break;
      }
    }
    if (player_id == -1)
    {
      printf("There is no player connected to socket: %d\n", client_socket);
    }

    /* try to add player to the queue */
    int32_t add_to_q_res = -1;
    if (header->player_id != -1)
    {
      printf("The player with id: %d already tried to join queue.\n", header->player_id);
    }
    else
    {
      printf("Adding player to queue...\n");
      add_to_q_res = addPlayerToQueue(req->username);
    }

    /* prepare join queue response (for the specific player only) */
    struct JoinQueueResponse *q_resp;
    q_resp = malloc(sizeof(struct JoinQueueResponse));
    if (add_to_q_res == -1)
    {
      q_resp->success = 0;
    }
    else
    {
      q_resp->success = 1;
      header->player_id = add_to_q_res;
    }

    printf("Created test objects\n");
    initDataServer();
    addJoinQueueResponseToData(q_resp, header->player_id, 1);

    /* extract the built packet and send to client */
    unsigned char *j_q_res = NULL;
    int j_q_packet_size = getHeaderSize() + getJoinQueueResponseSize() + getFooterSize();

    j_q_res = (unsigned char *)malloc(j_q_packet_size);
    if (j_q_res == NULL)
    {
      printf("Failed to alloc memory for JoinQueueResponse.\n");
    }
    int j_q_buffSize = getDataBufferServer(j_q_res, j_q_packet_size);
    send(client_socket, j_q_res, j_q_buffSize, 0);

    freeDataServer();
    free(q_resp);
    free(j_q_res);
    printf("Sent JoinQueueResponse to the client...\n");

    /* prepare queue status (for all players in queue) */
    struct QueueStatus *queue_status = getCurrentQueueStatus();

    initDataServer();
    player_id = -1;
    addQueueStatusToData(queue_status, player_id, 1);

    /* extract the built packet and send to client */
    unsigned char *q_s_res = NULL;
    int q_s_packet_size = getHeaderSize() + getQueueStatusSize(queue_status) + getFooterSize();

    q_s_res = (unsigned char *)malloc(q_s_packet_size);
    if (q_s_res == NULL)
    {
      printf("Failed to alloc memory for QueueStatus.\n");
    }
    int q_s_buffSize = getDataBufferServer(q_s_res, q_s_packet_size);

    printf("Buffer right before sending...\n");
    print_buffer_hex(q_s_res, q_s_packet_size);

    for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (client_data[i].client_socket != -1)
      {
        player_id = i;
        send(client_data[i].client_socket, q_s_res, q_s_buffSize, 0);
      }
    }

    freeDataServer();
    free(q_s_res);
    printf("Sent QueueStatus to the client...\n");
    break;
  }
  case BTN_PRESSED:
  {
    struct ButtonPressed *btn_pressed = getButtonPressed(buffer + localEnd);
    printf("\n");
    printf("ButtonPressed: \n");

    int i;
    for (i = 0; i < 5; i++)
    {
      printf("%c", btn_pressed->all_button_codes[i]);
    }
    printf("\n");

    localEnd += getButtonPressedSize();
    packet_data_size += getButtonPressedSize();

    if (getCurrentState() == BOARD_LOGIC_STATE)
    {
      printf("The game is over!\n");

      /* prepare game over response */
      initDataServer();
      int player_id = -1;
      addGameOverToData(player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *game_over_res = NULL;
      int game_over_packet_size = getHeaderSize() + getFooterSize();

      game_over_res = (unsigned char *)malloc(game_over_packet_size);
      if (game_over_res == NULL)
      {
        printf("Failed to alloc memory for GameReadyResponse.\n");
      }
      int game_over_buffSize = getDataBufferServer(game_over_res, game_over_packet_size);

      /* send to all players in game state */
      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, game_over_res, game_over_buffSize, 0);
        }
      }

      freeDataServer();
      free(game_over_res);
      printf("Sent GameOver to the client...\n");

      /* prepare scoreboard response */
      struct Scoreboard *board = getCurrentScoreboard();

      initDataServer();
      player_id = -1;
      addScoreboardToData(board, player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *board_res = NULL;
      int board_packet_size = getHeaderSize() + getScoreboardSize(board) + getFooterSize();

      board_res = (unsigned char *)malloc(board_packet_size);
      if (board_res == NULL)
      {
        printf("Failed to alloc memory for Scoreboard.\n");
      }
      int board_buffSize = getDataBufferServer(board_res, board_packet_size);

      /* send to all players in game state */
      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, board_res, board_buffSize, 0);
        }
      }

      freeDataServer();
      free(board_res);
      printf("Sent Scoreboard to the client...\n");
    }
    else
    {
      /* player movement logic */
      if (header->player_id != -1)
      {
        handlePlayerMove(header->player_id, btn_pressed->all_button_codes);
      }
      else
      {
        printf("This player is not in the game.\n");
      }

      /* prepare game state (for all players in game) */
      struct GameState *game_state = getCurrentGameState();

      initDataServer();
      int player_id = -1;
      addGameStateToData(game_state, player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *game_state_res = NULL;
      int game_state_packet_size = getHeaderSize() + getGameStateSize(game_state) + getFooterSize();

      game_state_res = (unsigned char *)malloc(game_state_packet_size);
      if (game_state_res == NULL)
      {
        printf("Failed to alloc memory for GameState.\n");
      }
      int game_state_buffSize = getDataBufferServer(game_state_res, game_state_packet_size);

      printf("Buffer right before sending...\n");
      print_buffer_hex(game_state_res, game_state_packet_size);

      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, game_state_res, game_state_buffSize, 0);
        }
      }

      freeDataServer();
      free(game_state_res);
      printf("Sent GameState to the client...\n");
    }
    break;
  }
  case LEAVE_QUEUE_REQ:
  {
    printf("Player left the queue \n");

    /* try to remove player from the queue */
    if (header->player_id == -1)
    {
      printf("This player is not in the queue.\n");
    }
    int32_t remove_from_q_res = removePlayerFromQueue(header->player_id);

    if (remove_from_q_res == -1)
    {
      printf("Failed to remove player from the queue.\n");
    }

    /*
    TODO: send updated QueueStatus to all players in queue
    */

    /* prepare queue status (for all players in queue) */
    struct QueueStatus *queue_status = getCurrentQueueStatus();

    initDataServer();
    int player_id = -1;
    addQueueStatusToData(queue_status, player_id, 1);

    /* extract the built packet and send to client */
    unsigned char *q_s_res = NULL;
    int q_s_packet_size = getHeaderSize() + getQueueStatusSize(queue_status) + getFooterSize();

    q_s_res = (unsigned char *)malloc(q_s_packet_size);
    if (q_s_res == NULL)
    {
      printf("Failed to alloc memory for QueueStatus.\n");
    }
    int q_s_buffSize = getDataBufferServer(q_s_res, q_s_packet_size);

    printf("Buffer right before sending...\n");
    print_buffer_hex(q_s_res, q_s_packet_size);

    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
      if (client_data[i].client_socket != -1)
      {
        player_id = i;
        send(client_data[i].client_socket, q_s_res, q_s_buffSize, 0);
      }
    }

    freeDataServer();
    free(q_s_res);
    break;
  }
  case PLAYER_READY_REQ:
  {
    printf("Player ready for game start \n");

    /* try to remove player from the queue */
    if (header->player_id == -1)
    {
      printf("This player is not in the queue.\n");
    }
    int32_t player_ready_res = markPlayerReady(header->player_id);

    if (player_ready_res == -1)
    {
      printf("Failed to mark player ready in the queue.\n");
    }

    /* if all players ready, notify about game start*/
    if (player_ready_res == -3)
    {
      printf("All players ready, starting the game...\n");

      /* prepare game ready response */
      initDataServer();
      int player_id = -1;
      addGameReadyResponseToData(player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *game_ready_res = NULL;
      int game_ready_packet_size = getHeaderSize() + getFooterSize();

      game_ready_res = (unsigned char *)malloc(game_ready_packet_size);
      if (game_ready_res == NULL)
      {
        printf("Failed to alloc memory for GameReadyResponse.\n");
      }
      int game_ready_buffSize = getDataBufferServer(game_ready_res, game_ready_packet_size);

      /* send to all players in queue */
      int i;
      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, game_ready_res, game_ready_buffSize, 0);
        }
      }

      freeDataServer();
      free(game_ready_res);

      /* prepare initial locations response */
      struct InitialLocations *init_locations = getCurrentMapLocations();

      initDataServer();
      player_id = -1;
      addInitialLocationsToData(init_locations, player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *init_loc_res = NULL;
      int init_loc_packet_size = getHeaderSize() + getInitialLocationsSize(init_locations) + getFooterSize();

      init_loc_res = (unsigned char *)malloc(init_loc_packet_size);
      if (init_loc_res == NULL)
      {
        printf("Failed to alloc memory for InitialLocations.\n");
      }
      int init_loc_buffSize = getDataBufferServer(init_loc_res, init_loc_packet_size);

      printf("Buffer right before sending...\n");
      print_buffer_hex(init_loc_res, init_loc_packet_size);

      /* send to all players in queue */
      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, init_loc_res, init_loc_buffSize, 0);
        }
      }

      freeDataServer();
      free(init_loc_res);

      /* prepare game state response */
      struct GameState *game_state = getCurrentGameState();

      initDataServer();
      player_id = -1;
      addGameStateToData(game_state, player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *game_state_res = NULL;
      int game_state_packet_size = getHeaderSize() + getGameStateSize(game_state) + getFooterSize();

      game_state_res = (unsigned char *)malloc(game_state_packet_size);
      if (game_state_res == NULL)
      {
        printf("Failed to alloc memory for GameState.\n");
      }
      int game_state_buffSize = getDataBufferServer(game_state_res, game_state_packet_size);

      printf("Buffer right before sending...\n");
      print_buffer_hex(game_state_res, game_state_packet_size);

      /* send to all players in queue */
      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, game_state_res, game_state_buffSize, 0);
        }
      }

      freeDataServer();
      free(game_state_res);
    }
    else
    {
      /* prepare queue status (for all players in queue) */
      struct QueueStatus *queue_status = getCurrentQueueStatus();

      initDataServer();
      int player_id = -1;
      addQueueStatusToData(queue_status, player_id, 1);

      /* extract the built packet and send to client */
      unsigned char *q_s_res = NULL;
      int q_s_packet_size = getHeaderSize() + getQueueStatusSize(queue_status) + getFooterSize();

      q_s_res = (unsigned char *)malloc(q_s_packet_size);
      if (q_s_res == NULL)
      {
        printf("Failed to alloc memory for QueueStatus.\n");
      }
      int q_s_buffSize = getDataBufferServer(q_s_res, q_s_packet_size);

      printf("Buffer right before sending...\n");
      print_buffer_hex(q_s_res, q_s_packet_size);

      int i;
      for (i = 0; i < MAX_CLIENTS; i++)
      {
        if (client_data[i].client_socket != -1)
        {
          player_id = i;
          send(client_data[i].client_socket, q_s_res, q_s_buffSize, 0);
        }
      }

      freeDataServer();
      free(q_s_res);
    }
    break;
  }
  default:
    printf("Unknown packet type: %d\n", header->msg_type);
    break;
  }

  struct Footer *footer = getFooter(buffer + localEnd);
  localEnd += getFooterSize();

  /*
  TODO: pass proper values for checksum calculation
  */
  if (footer->checksum != calcCRC(buffer + getHeaderSize(), packet_data_size))
  {
    printf("Incorrect checksum, invalid packet: %d\n", footer->checksum);
  }

  /*
  TODO: handle if packet is not the last in the message
  */
  if (footer->end != 4)
  {
    printf("Packet content not finished yet \n");
  }
}

void server_cleanup()
{
  /*
  TODO: for testing only, reorganize for production
  TODO: fix segfault on queue clean up
  */

  /* deleteQueue(); */
  deleteGameMapLocations();
  deleteGameState();
  deleteScoreboard();
  printf("Server logic cleaned.\n");
}

void server_sigint_handler(int sig)
{
  (void)sig;
  printf("Ctrl+C detected. Clean up...\n");
  server_cleanup();
  exit(EXIT_SUCCESS);
}

/*
! Temporary solution for testing
*/
void test_receive_packet_from_file_server(const char *filename)
{
  FILE *file = fopen(filename, "rb");
  if (file == NULL)
  {
    perror("Unable to open file");
    return;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  unsigned char *buffer = malloc(fileSize);
  if (buffer == NULL)
  {
    printf("Memory allocation failed\n");
    fclose(file);
    return;
  }

  fread(buffer, 1, fileSize, file);
  fclose(file);

  print_buffer_hex(buffer, fileSize);
  parse_packet_server(buffer, fileSize, 0);
  free(buffer);
}