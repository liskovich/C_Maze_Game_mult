#include "helpers_common.h"
#include "packets_common.h"
#include "packets_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>

/* graphics imports */
#include <GL/glut.h>
#include <SOIL/SOIL.h>

#include "view.player.h"
#include "view.objects.h"
#include "view.tile.h"

#define UNUSED(x) (void)(x)
#define MAX_MAZE_WIDTH 15
#define MAX_MAZE_HEIGHT 15
#define SCREEN_REDRAW_FREQUENCY_MILLIS 16

#define PLAYER_WIDTH 2.0f
#define PLAYER_HEIGHT 2.0f

#define QUEUE_LOGIC_STATE_CLIENT 1
#define GAME_LOGIC_STATE_CLIENT 2
#define BOARD_LOGIC_STATE_CLIENT 3

#define BUFFER_SIZE (1024 * 1024)

#define CLIENT_START_FAIL_ERR 1
#define STD_IN 0
#define STD_OUT 1

void start_client(char *ip, int port);
void parse_packet_client(unsigned char *buffer, int size);
void test_receive_packet_from_file_client(const char *filename);
static void *server_handler(void *arg);
static void *graphics_handler(void *arg);

void send_join_queue_request(char *player_username);
void send_leave_queue_request();
void send_player_ready_request();
void send_button_pressed_request(char *btn_code);

/* graphics functions */
int display_x = 0;
int display_y = 9;

int flip_horizontal = 0;

int mainWindow;

void renderBitmapString(float x, float y, void *font, const char *string);
void initOpenGL();
void drawQueueStateClient();
void drawGridClient();
void drawGameStateClient();
void drawScoreboardClient();
void handleMovementClient(unsigned char key, int x, int y);
void reshapeClient(int width, int height);
void displayUI();
void display_ingame_score();

/* general game variables */
int current_gamer_id = -1;
int client_socket = -1;

int actual_player_count = 0;
int current_game_state = QUEUE_LOGIC_STATE_CLIENT;

/*
TODO: add variables for: q_status, init_loc, game_state and scoreboard
update them on data receipt from server
*/
struct QueueStatus *q_status_client_GLOBAL = NULL;
struct InitialLocations *init_locations_client_GLOBAL = NULL;
struct GameState *game_state_client_GLOBAL = NULL;
struct Scoreboard *scoreboard_client_GLOBAL = NULL;

void initClientQueueStatus();
void updateClientQueueStatus(struct QueueStatus *q_status);
void cleanClientQueueStatus();

void initClientMapLocations();
void updateClientMapLocations(struct InitialLocations *locations);
void cleanClientMapLocations();

// void initClientGameState();
void updateClientGameState(struct GameState *game_state);
void cleanClientGameState();

void initClientScoreboard(struct Scoreboard *board);
void cleanClientScoreboard();

void triggerGraphicsRedraw(int value);

void clientStatesCleanUp();
void client_sigint_handler(int sig);

int main(int argc, char **argv)
{
  /* parse the ip and port provided in args */
  if (argc < 3)
  {
    printf("Provide IP and port:\n %s -a=<IP_ADDRESS> -p=<PORT_NUMBER>\n", argv[0]);
    return CLIENT_START_FAIL_ERR;
  }

  char *ip_address = strstr(argv[1], "=") + 1;
  int port = atoi(strstr(argv[2], "=") + 1);

  /* replace file path with the one relevant for you */
  /* test_receive_packet_from_file_client("scoreboard_output.bin"); */
  /* test_receive_packet_from_file_client("join_queue_resp_output.bin"); */
  /* test_receive_packet_from_file_client("queue_status_output.bin"); */
  /* test_receive_packet_from_file_client("initial_locations_output.bin"); */
  /* test_receive_packet_from_file_client("notification_output.bin"); */
  /* test_receive_packet_from_file_client("game_state_output.bin"); */

  /* init game state variables */
  initClientQueueStatus();
  initClientMapLocations();

  /* add signal handlers */
  atexit(clientStatesCleanUp);
  signal(SIGINT, client_sigint_handler);

  start_client(ip_address, port);
  return 0;
}

void start_client(char *ip, int port)
{
  /* set up server address */
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  struct hostent *host = gethostbyname(ip);
  if (host == NULL)
  {
    printf("Host get error\n");
    exit(CLIENT_START_FAIL_ERR);
  }

  inet_pton(AF_INET, host->h_name, &server_addr.sin_addr);

  /* create client socket */
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (client_socket == -1)
  {
    printf("Socket error\n");
    exit(CLIENT_START_FAIL_ERR);
  }

  /* connect to server */
  if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Connection to server error\n");
    exit(CLIENT_START_FAIL_ERR);
  }

  /* separate thread for network process */
  pthread_t thread;
  if (pthread_create(&thread, NULL, server_handler, &client_socket) != 0)
  {
    printf("Thread create error\n");
    close(client_socket);
    exit(CLIENT_START_FAIL_ERR);
  }

  /* separate thread for graphics process */
  pthread_t g_thread;
  int arg_count = 1;
  if (pthread_create(&g_thread, NULL, graphics_handler, &arg_count) != 0)
  {
    printf("Thread create error (graphics)\n");
    close(client_socket);
    exit(CLIENT_START_FAIL_ERR);
  }

  /*
  TODO: create a separate thread for UI (graphics)
  */

  /*
  ! FOR TESTING PURPOSES ONLY
  */
  char message[BUFFER_SIZE];
  while (fgets(message, sizeof(message), stdin))
  {
    /* read user input */
    message[strcspn(message, "\n")] = '\0';

    if (strcmp(message, "j") == 0)
    {
      printf("Selected message: join\n");
      send_join_queue_request("ricards");
    }

    /* movements */
    else if (strcmp(message, "w") == 0)
    {
      printf("Selected message: press W\n");
      send_button_pressed_request("w");
    }
    else if (strcmp(message, "s") == 0)
    {
      printf("Selected message: press S\n");
      send_button_pressed_request("s");
    }
    else if (strcmp(message, "a") == 0)
    {
      printf("Selected message: press A\n");
      send_button_pressed_request("a");
    }
    else if (strcmp(message, "d") == 0)
    {
      printf("Selected message: press D\n");
      send_button_pressed_request("d");
    }
    /* end of movements */

    else if (strcmp(message, "l") == 0)
    {
      printf("Selected message: leave\n");
      send_leave_queue_request();
    }
    else if (strcmp(message, "r") == 0)
    {
      printf("Selected message: ready\n");
      send_player_ready_request();
    }
    else
    {
      printf("Selected command not found.\n");
    }
  }

  /* cleanup */
  pthread_join(thread, NULL);
  pthread_join(g_thread, NULL);
  close(client_socket);
}

void send_join_queue_request(char *player_username)
{
  /* init packet buffer */
  int packet_size = getHeaderSize() + getJoinQueueRequestSize() + getFooterSize();
  unsigned char *buffer = malloc(packet_size);

  /* build packet object */
  struct JoinQueueRequest *q_req;
  q_req = malloc(sizeof(struct JoinQueueRequest));
  strcpy(q_req->username, player_username);
  printf("Created test objects\n");

  initDataClient();
  addJoinQueueRequest(q_req, current_gamer_id, 1);

  /* extract the built packet and send to server */
  if (buffer == NULL)
  {
    printf("Failed to alloc memory for JoinQueueResponse.\n");
  }
  int buffer_size = getDataBufferClient(buffer, packet_size);
  if (client_socket != -1)
  {
    send(client_socket, buffer, buffer_size, 0);
  }

  /* cleanup */
  freeDataClient();
  free(q_req);
  free(buffer);
}

void send_leave_queue_request()
{
  if (current_gamer_id == -1)
  {
    printf("The player is not in the queue.\n");
    return;
  }

  /* init packet buffer */
  int packet_size = getHeaderSize() + getFooterSize();
  unsigned char *buffer = malloc(packet_size);

  initDataClient();
  addLeaveQueueRequest(current_gamer_id, 1);

  /* extract the built packet and send to server */
  if (buffer == NULL)
  {
    printf("Failed to alloc memory for LeaveQueueRequest.\n");
  }
  int buffer_size = getDataBufferClient(buffer, packet_size);
  if (client_socket != -1)
  {
    send(client_socket, buffer, buffer_size, 0);
  }

  /* cleanup */
  freeDataClient();
  free(buffer);

  /* show that player is not in the queue anymore */
  current_gamer_id = -1;
}

void send_player_ready_request()
{
  if (current_gamer_id == -1)
  {
    printf("The player is not in the queue.\n");
    return;
  }

  /* init packet buffer */
  int packet_size = getHeaderSize() + getFooterSize();
  unsigned char *buffer = malloc(packet_size);

  initDataClient();
  addPlayerReadyRequest(current_gamer_id, 1);

  /* extract the built packet and send to server */
  if (buffer == NULL)
  {
    printf("Failed to alloc memory for PlayerReadyRequest.\n");
  }
  int buffer_size = getDataBufferClient(buffer, packet_size);
  if (client_socket != -1)
  {
    send(client_socket, buffer, buffer_size, 0);
  }

  /* cleanup */
  freeDataClient();
  free(buffer);
}

void send_button_pressed_request(char *btn_code)
{
  if (current_gamer_id == -1)
  {
    printf("The player is not in the game.\n");
    return;
  }

  /* init packet buffer */
  int packet_size = getHeaderSize() + getButtonPressedSize() + getFooterSize();
  unsigned char *buffer = malloc(packet_size);

  /* build packet object */
  struct ButtonPressed *btn_pressed;
  btn_pressed = malloc(sizeof(struct ButtonPressed));
  memcpy(btn_pressed->all_button_codes, btn_code, 5 * sizeof(char));
  printf("Created test objects\n");

  initDataClient();
  addButtonPressed(btn_pressed, current_gamer_id, 1);

  /* extract the built packet and send to server */
  if (buffer == NULL)
  {
    printf("Failed to alloc memory for ButtonPressed.\n");
  }
  int buffer_size = getDataBufferClient(buffer, packet_size);
  if (client_socket != -1)
  {
    send(client_socket, buffer, buffer_size, 0);
  }

  /* cleanup */
  freeDataClient();
  free(btn_pressed);
  free(buffer);
}

void parse_packet_client(unsigned char *buffer, int size)
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
  case JOIN_QUEUE_RESP:
  {
    struct JoinQueueResponse *resp = getJoinQueueResponse(buffer + localEnd);
    printf("\n");
    printf("JoinQueueResponse: %s\n", resp->success ? "Success" : "Failure");

    localEnd += getJoinQueueResponseSize();
    packet_data_size += getJoinQueueResponseSize();

    if (current_gamer_id == -1)
    {
      current_gamer_id = header->player_id;
      printf("Assigned gamer_id: %d to player.\n", current_gamer_id);
    }

    /*
    TODO: for the player who connected, call function to display success message
    */
    break;
  }
  case QUEUE_STATUS:
  {
    struct QueueStatus *status = getQueueStatus(buffer + localEnd);

    /*
    ! IMPORTANT: the server will always send some fixed amount of players in queue status (e.g. 10, 15, 20)
    ! this amount will be available inside "status->players_count" and DOES NOT represent the actual real amount of players
    ! in order to check if a player in the queue is real or just a placeholder, check for: 1) username presence 2) gamer_id value (should not be -1)
    ! update the number of players (displayed in the queue state) based on the actual real player count (update "actual_player_count")
    */
    printf("\n");
    printf("Players count is: %d\n", status->players_count);

    /* display full queue status info */
    int i;
    for (i = 0; i < status->players_count; i++)
    {
      printf("Player %d username is: ", i);
      int j;
      for (j = 0; j < 32; j++)
      {
        printf("%c", status->players_in_queue[i]->username[j]);
      }
      printf("\n");

      printf("Player %d gamer_id is: %d\n", i, status->players_in_queue[i]->gamer_id);
      printf("Player %d is_ready is: %d\n", i, status->players_in_queue[i]->is_ready);
      printf("\n");
    }

    localEnd += getQueueStatusSize(status);
    packet_data_size += getQueueStatusSize(status);

    updateClientQueueStatus(status);
    // displayUI();
    triggerGraphicsRedraw(0);
    /*
    TODO: redraw UI here
    */
    break;
  }
  case INIT_LOCATIONS:
  {
    struct InitialLocations *locations = getInitialLocations(buffer + localEnd);
    printf("\n");
    printf("Map height %d\n", locations->map_height);
    printf("Map width %d\n", locations->map_width);
    printf("\nTiles:\n");
    int i;
    for (i = 0; i < locations->map_height; i++)
    {
      int idx = i * locations->map_width;
      int j;
      for (j = idx; j < locations->map_width + idx; j++)
      {
        printf("%c ", locations->tiles[j]);
      }
      printf("\n");
    }

    localEnd += getInitialLocationsSize(locations);
    packet_data_size += getInitialLocationsSize(locations);

    /* drawGridClient(locations); */
    updateClientMapLocations(locations);
    // displayUI();
    triggerGraphicsRedraw(0);
    /*
    TODO: redraw UI here
    */
    break;
  }
  case GAME_READY_RESP:
  {
    printf("The game has started \n");

    /*
    TODO: for the players who connected, call function to display "game started" message
    */
    current_game_state = GAME_LOGIC_STATE_CLIENT;
    // displayUI();
    triggerGraphicsRedraw(0);
    /*
    TODO: redraw UI here
    */
    break;
  }
  case GAME_STATE:
  {
    struct GameState *state = getGameState(buffer + localEnd);
    printf("\n");
    printf("Players count is: %d\n", state->players_count);
    printf("Objects count is: %d\n", state->objects_count);

    /* display full game state info */
    int i;
    for (i = 0; i < state->players_count; i++)
    {
      printf("Player %d username is: ", i);
      int j;
      for (j = 0; j < 32; j++)
      {
        printf("%c", state->players[i]->username[j]);
      }
      printf("\n");

      printf("Player %d gamer_id is: %d\n", i, state->players[i]->gamer_id);
      printf("Player %d x is: %f\n", i, state->players[i]->x);
      printf("Player %d y is: %f\n", i, state->players[i]->y);
      printf("Player %d score is: %f\n", i, state->players[i]->score);
      printf("\n");
    }

    for (i = 0; i < state->objects_count; i++)
    {
      printf("Object %d id is: %d\n", i, state->objects[i]->id);
      printf("Object %d type is: %c\n", i, state->objects[i]->type);
      printf("Object %d x is: %f\n", i, state->objects[i]->x);
      printf("Object %d y is: %f\n", i, state->objects[i]->y);
      printf("\n");
    }

    localEnd += getGameStateSize(state);
    packet_data_size += getGameStateSize(state);

    /* drawGameStateClient(state); */
    updateClientGameState(state);
    // displayUI();
    triggerGraphicsRedraw(0);
    /*
    TODO: redraw UI here
    */
    break;
  }
  case NOTIFICATION:
  {
    struct Notification *note = getNotification(buffer + localEnd);
    printf("\n");
    printf("Notification: \n");

    int i;
    for (i = 0; i < 256; i++)
    {
      printf("%c", note->message[i]);
    }
    printf("\n");

    localEnd += getNotificationSize();
    packet_data_size += getNotificationSize();

    /*
    TODO: call function to display a notification
    */
    break;
  }
  case GAME_OVER:
  {
    printf("The game has finished \n");

    /*
    TODO: for the players who connected, call function to display "game over" message
    */
    current_game_state = BOARD_LOGIC_STATE_CLIENT;
    // displayUI();
    triggerGraphicsRedraw(0);
    /*
    TODO: redraw UI here
    */
    break;
  }
  case SCORE_BOARD:
  {
    struct Scoreboard *board = getScoreboard(buffer + localEnd);
    printf("\n");
    printf("Players count is: %d\n", board->players_count);

    /* display full scoreboard info */
    int i;
    for (i = 0; i < board->players_count; i++)
    {
      printf("Player %d username is: ", i);
      int j;
      for (j = 0; j < 32; j++)
      {
        printf("%c", board->players[i]->username[j]);
      }
      printf("\n");

      printf("Player %d gamer_id is: %d\n", i, board->players[i]->gamer_id);
      printf("Player %d score is: %f\n", i, board->players[i]->score);
      printf("Player %d stats_count is: %d\n", i, board->players[i]->stats_count);

      /* print stats list for each player */
      for (j = 0; j < board->players[i]->stats_count; j++)
      {
        int idx = j + j * 127;
        printf("Player %d stat nr. %d is: ", i, j);
        int k;
        for (k = idx; k < idx + 128; k++)
        {
          printf("%c", board->players[i]->stats_info[k]);
        }
        printf("\n");
      }
      printf("\n");
    }

    localEnd += getScoreboardSize(board);
    packet_data_size += getScoreboardSize(board);

    /*
    TODO: call function to draw scoreboard
    */
    initClientScoreboard(board);
    // displayUI();
    triggerGraphicsRedraw(0);
    /*
    TODO: redraw UI here
    */
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

static void *server_handler(void *arg)
{
  int client_socket = *(int *)arg;
  unsigned char buffer[BUFFER_SIZE];

  /* read data from server */
  while (1)
  {
    int bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read <= 0)
    {
      break;
    }

    /* parse server data */
    parse_packet_client(buffer, BUFFER_SIZE);
  }

  close(client_socket);
  pthread_exit(NULL);
}

static void *graphics_handler(void *arg)
{
  /* OpenGL code here */
  int dummy_argc = *(int *)arg;

  // char *dummy_argv[] = {"client.exe", NULL};
  char *dummy_argv[1];
  dummy_argv[0] = "client.exe";
  dummy_argv[1] = NULL;
  glutInit(&dummy_argc, dummy_argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(800, 800);
  mainWindow = glutCreateWindow("2-Color Tile Maze");

  initOpenGL();

  glutDisplayFunc(displayUI);
  glutReshapeFunc(reshapeClient);
  glutKeyboardFunc(handleMovementClient);

  loadPlayerTexture("./sprites/characters/player.png");
  loadCoinTexture("./sprites/objects/coin.png");

  char *wallTexture = "./sprites/tiles/wall.png";
  char *floorTexture = "./sprites/tiles/floor.png";
  loadTileTextures(wallTexture, floorTexture);

  /* redraw the screen at approx 60 FPS */
  glutTimerFunc(SCREEN_REDRAW_FREQUENCY_MILLIS, triggerGraphicsRedraw, 0);

  glutMainLoop();

  pthread_exit(NULL);
}

/* graphics functions implementation */
void initOpenGL()
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, MAX_MAZE_WIDTH, 0.0, MAX_MAZE_HEIGHT);
}

void renderBitmapString(float x, float y, void *font, const char *string)
{
  const char *c;
  glRasterPos2f(x, y);
  for (c = string; *c != '\0'; c++)
  {
    glutBitmapCharacter(font, *c);
  }
}

/*
TODO - new function drawing main menu
It works all right
Press J, then R, then any walking button
*/
void drawQueueStateClient()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 800, 0, 800);
  glScalef(1, -1, 1);
  glTranslatef(0, -800, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glColor3f(1.0, 1.0, 1.0); /* Text color */

  /* Text */
  renderBitmapString(800 / 2 - 50, 800 / 2 - 10, GLUT_BITMAP_TIMES_ROMAN_24, "Superior maze coin game");
  renderBitmapString(50, 800 - 100, GLUT_BITMAP_9_BY_15, "press 'J' to join queue");
  renderBitmapString(50, 800 - 50, GLUT_BITMAP_9_BY_15, "press 'L' to exit");

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();
}

void drawGridClient()
{
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);

  int i;
  for (i = 0; i < init_locations_client_GLOBAL->map_height; i++)
  {
    int idx = i * init_locations_client_GLOBAL->map_width;
    int j;
    for (j = idx; j < init_locations_client_GLOBAL->map_width + idx; j++)
    {
      int tile_type = -1;
      if (init_locations_client_GLOBAL->tiles[j] == '.')
      {
        tile_type = 0;
      }
      if (init_locations_client_GLOBAL->tiles[j] == '|')
      {
        tile_type = 1;
      }

      int usual_idx = j - (i * init_locations_client_GLOBAL->map_width);
      drawTile(usual_idx, init_locations_client_GLOBAL->map_height - i - 1, tile_type);
    }
  }

  glDisable(GL_TEXTURE_2D);
  glFlush();
}

void drawGameStateClient()
{
  if (game_state_client_GLOBAL != NULL)
  {
    int i;
    for (i = 0; i < game_state_client_GLOBAL->players_count; i++)
    {
      /*
      TODO: add user-specific rotation animation
      */

      /* adjust color to identify current player */
      int player_color = 1;
      if (game_state_client_GLOBAL->players[i]->gamer_id == current_gamer_id)
      {
        player_color = 0;
      }

      drawPlayer(
          game_state_client_GLOBAL->players[i]->x,
          game_state_client_GLOBAL->players[i]->y,
          PLAYER_WIDTH,
          PLAYER_HEIGHT,
          0, /* display_x */
          9, /* display_y */
          0, /* flip_horizontal */
          player_color);
    }
    for (i = 0; i < game_state_client_GLOBAL->objects_count; i++)
    {
      drawCoin(
          game_state_client_GLOBAL->objects[i]->id,
          game_state_client_GLOBAL->objects[i]->type,
          game_state_client_GLOBAL->objects[i]->x,
          game_state_client_GLOBAL->objects[i]->y);
    }
  }
}
/*
TODO - new function drawScoreBoardClient.
What is not warking - reading scoreboard packet, so it reads gmestate temporarily
*/
void drawScoreboardClient()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 800, 0, 800);
  glScalef(1, -1, 1);
  glTranslatef(0, -800, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glColor3f(1.0, 1.0, 1.0); /* White color text */

  renderBitmapString(800 / 2 - 50, 800 / 2 - 50, GLUT_BITMAP_TIMES_ROMAN_24, "Scoreboard");
  int i;
  float yOffset = 50.0f; /* Upper left corner */
  for (i = 0; i < scoreboard_client_GLOBAL->players_count; i++)
  {
    char info[50];
    snprintf(info, sizeof(info), "%s:         %.2f", scoreboard_client_GLOBAL->players[i]->username, scoreboard_client_GLOBAL->players[i]->score);
    renderBitmapString(300.0f, 400.0f + yOffset * i, GLUT_BITMAP_TIMES_ROMAN_24, info);
  }
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();
}

void handleMovementClient(unsigned char key, int x, int y)
{
  /*
   * Unused
   */
  UNUSED(x);
  UNUSED(y);

  switch (key)
  {
  case 'w':
    /* Up */
    printf("Go UP: W\n");
    send_button_pressed_request("w");

    flip_horizontal = 0;
    if (display_y != 7)
    {
      display_y = 7;
    }
    break;
  case 's':
    /* Down */
    printf("Go DOWN: S\n");
    send_button_pressed_request("s");

    flip_horizontal = 0;
    if (display_y != 9)
    {
      display_y = 9;
    }
    break;
  case 'a':
    /* Left */
    printf("Go LEFT: A\n");
    send_button_pressed_request("a");

    flip_horizontal = 1;
    if (display_y != 8)
    {
      display_y = 8;
    }
    break;
  case 'd':
    /* Right */
    printf("Go RIGHT: D\n");
    send_button_pressed_request("d");

    flip_horizontal = 0;
    if (display_y != 8)
    {
      display_y = 8;
    }
    break;
  /*
  TODO: the options below should be disabled in game state
  */
  case 'j':
    /* join queue */
    /*
    TODO: allow for username input
    */
    printf("Join QUEUE: J\n");
    send_join_queue_request("ricards");
    break;
  case 'r':
    /* ready */
    printf("Ready QUEUE: R\n");
    send_player_ready_request();
    break;
  case 'l':
    /* leave queue */
    printf("Leave QUEUE: L\n");
    send_leave_queue_request();
    break;
  }

  glutPostRedisplay();
}

void reshapeClient(int width, int height)
{
  int size = (width > height) ? height : width;

  int x = (width - size) / 2;
  int y = (height - size) / 2;

  glViewport(x, y, size, size);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, MAX_MAZE_WIDTH, 0.0, MAX_MAZE_HEIGHT);
  glMatrixMode(GL_MODELVIEW);
}

/*
TODO - new function for ingame score
Test for multiple players
*/

void display_ingame_score()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, 800, 0, 800);
  glScalef(1, -1, 1);
  glTranslatef(0, -800, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glColor3f(1.0, 1.0, 1.0); /* White color text */

  int i;
  float yOffset = 20.0f; /* Upper left corner */
  for (i = 0; i < game_state_client_GLOBAL->players_count; i++)
  {
    char info[50];
    snprintf(info, sizeof(info), "%s: %.2f", game_state_client_GLOBAL->players[i]->username, game_state_client_GLOBAL->players[i]->score);
    renderBitmapString(10.0f, 30.0f + yOffset * i, GLUT_BITMAP_TIMES_ROMAN_24, info);
  }
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glFlush();
}

void displayUI()
{
  glClear(GL_COLOR_BUFFER_BIT);

  /* draw queue or scoreboard */
  if (current_game_state == QUEUE_LOGIC_STATE_CLIENT)
  {
    drawQueueStateClient();
  }
  if (current_game_state == BOARD_LOGIC_STATE_CLIENT)
  {
    drawScoreboardClient();
  }

  /* draw maze, players and objects */
  if (game_state_client_GLOBAL != NULL && current_game_state == GAME_LOGIC_STATE_CLIENT)
  {
    drawGridClient();
    drawGameStateClient();
    display_ingame_score();
  }

  glutSwapBuffers();
}

/* state variable operations */
void initClientQueueStatus()
{
  /* memory for the QueueStatus */
  q_status_client_GLOBAL = (struct QueueStatus *)malloc(sizeof(struct QueueStatus));
  if (q_status_client_GLOBAL == NULL)
  {
    exit(EXIT_FAILURE);
  }

  q_status_client_GLOBAL->players_count = 4;
  /* memory for players_in_queue array */
  q_status_client_GLOBAL->players_in_queue = (struct PlayerQueueInfo **)malloc(q_status_client_GLOBAL->players_count * sizeof(struct PlayerQueueInfo *));
  if (q_status_client_GLOBAL->players_in_queue == NULL)
  {
    free(q_status_client_GLOBAL);
    exit(EXIT_FAILURE);
  }

  /* init each player info */
  int i;
  for (i = 0; i < q_status_client_GLOBAL->players_count; i++)
  {
    q_status_client_GLOBAL->players_in_queue[i] = (struct PlayerQueueInfo *)malloc(sizeof(struct PlayerQueueInfo));
    if (q_status_client_GLOBAL->players_in_queue[i] == NULL)
    {
      int j;
      for (j = 0; j < i; j++)
      {
        free(q_status_client_GLOBAL->players_in_queue[j]);
      }
      free(q_status_client_GLOBAL->players_in_queue);
      free(q_status_client_GLOBAL);
      exit(EXIT_FAILURE);
    }

    /* set dummy values for player */
    char empty_string[32] = "";
    memcpy(q_status_client_GLOBAL->players_in_queue[i]->username, empty_string, 32 * sizeof(char));
    q_status_client_GLOBAL->players_in_queue[i]->gamer_id = -1;
    q_status_client_GLOBAL->players_in_queue[i]->is_ready = 0;
  }
}

void updateClientQueueStatus(struct QueueStatus *q_status)
{
  cleanClientQueueStatus();
  q_status_client_GLOBAL = q_status;
}

void cleanClientQueueStatus()
{
  if (q_status_client_GLOBAL != NULL)
  {
    int i;
    for (i = 0; i < q_status_client_GLOBAL->players_count; i++)
    {
      free(q_status_client_GLOBAL->players_in_queue[i]);
    }
    free(q_status_client_GLOBAL->players_in_queue);
    free(q_status_client_GLOBAL);
    q_status_client_GLOBAL = NULL;
  }
}

void initClientMapLocations()
{
  /* memory for the InitialLocations */
  init_locations_client_GLOBAL = (struct InitialLocations *)malloc(sizeof(struct InitialLocations));
  if (init_locations_client_GLOBAL == NULL)
  {
    exit(EXIT_FAILURE);
  }

  init_locations_client_GLOBAL->map_width = MAX_MAZE_WIDTH;
  init_locations_client_GLOBAL->map_height = MAX_MAZE_HEIGHT;

  /* memory for tiles */
  init_locations_client_GLOBAL->tiles = (char *)malloc(init_locations_client_GLOBAL->map_width * init_locations_client_GLOBAL->map_height * sizeof(char));
  if (init_locations_client_GLOBAL->tiles == NULL)
  {
    free(init_locations_client_GLOBAL);
    exit(EXIT_FAILURE);
  }

  /* set each tile to '.' */
  int i;
  for (i = 0; i < init_locations_client_GLOBAL->map_width * init_locations_client_GLOBAL->map_height; i++)
  {
    init_locations_client_GLOBAL->tiles[i] = '.';
  }
}

void updateClientMapLocations(struct InitialLocations *locations)
{
  cleanClientMapLocations();
  init_locations_client_GLOBAL = locations;
}

void cleanClientMapLocations()
{
  if (init_locations_client_GLOBAL != NULL)
  {
    free(init_locations_client_GLOBAL->tiles);
    free(init_locations_client_GLOBAL);
    init_locations_client_GLOBAL = NULL;
  }
}

// void initClientGameState();
void updateClientGameState(struct GameState *game_state)
{
  cleanClientGameState();
  game_state_client_GLOBAL = game_state;
}

void cleanClientGameState()
{
  if (game_state_client_GLOBAL != NULL)
  {
    int i;
    /* free players memory */
    for (i = 0; i < game_state_client_GLOBAL->players_count; i++)
    {
      free(game_state_client_GLOBAL->players[i]);
    }
    free(game_state_client_GLOBAL->players);

    /* free objects memory */
    for (i = 0; i < game_state_client_GLOBAL->objects_count; i++)
    {
      free(game_state_client_GLOBAL->objects[i]);
    }
    free(game_state_client_GLOBAL->objects);

    free(game_state_client_GLOBAL);
    game_state_client_GLOBAL = NULL;
  }
}

void initClientScoreboard(struct Scoreboard *board)
{
  scoreboard_client_GLOBAL = board;
}

void cleanClientScoreboard()
{
  if (scoreboard_client_GLOBAL != NULL)
  {
    int i;
    for (i = 0; i < scoreboard_client_GLOBAL->players_count; i++)
    {
      free(scoreboard_client_GLOBAL->players[i]->stats_info);
      free(scoreboard_client_GLOBAL->players[i]);
    }
    free(scoreboard_client_GLOBAL->players);

    free(scoreboard_client_GLOBAL);
    scoreboard_client_GLOBAL = NULL;
  }
}

void triggerGraphicsRedraw(int value)
{
  (void)value;
  glutPostWindowRedisplay(mainWindow);
}

void clientStatesCleanUp()
{
  cleanClientQueueStatus();
  cleanClientMapLocations();
  cleanClientGameState();
  cleanClientScoreboard();
}

void client_sigint_handler(int sig)
{
  (void)sig;
  printf("Ctrl+C detected. Clean up...\n");
  clientStatesCleanUp();
  exit(EXIT_SUCCESS);
}

/*
! Temporary solution for testing
*/
void test_receive_packet_from_file_client(const char *filename)
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
  parse_packet_client(buffer, fileSize);
  free(buffer);
}