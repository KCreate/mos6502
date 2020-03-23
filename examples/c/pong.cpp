/*
 * Pong game written for the MOS 6502 Emulator Project
 * Author: Leonard Schütz
 *
 * This game was developed as my BMA-Project for the Berufsmaturitätsschule in Zürich.
 *
 * */

/* Important Addresses */
static constexpr uint8_t* ADDR_CONTROL = 0x4900;
static constexpr uint8_t* ADDR_TEXT_MODE_BACKGROUND_COLOR = 0x4901;
static constexpr uint8_t* ADDR_TEXT_MODE_FOREGROUND_COLOR = 0x4902;
static constexpr uint8_t* ADDR_EVENT_TYPE = 0x4903;
static constexpr uint8_t* ADDR_EVENT_ARG1 = 0x4904;
static constexpr uint8_t* ADDR_EVENT_ARG2 = 0x4905;
static constexpr uint8_t* ADDR_CLOCK1 = 0x4906;
static constexpr uint8_t* ADDR_CLOCK2 = 0x4907;
static constexpr uint8_t* ADDR_AUDIO1 = 0x4908;
static constexpr uint8_t* ADDR_AUDIO2 = 0x4909;
static constexpr uint8_t* ADDR_AUDIO3 = 0x490A;
static constexpr uint8_t* ADDR_DRAW_METHOD = 0x490B;
static constexpr uint8_t* ADDR_DRAW_ARG1 = 0x490C;
static constexpr uint8_t* ADDR_DRAW_ARG2 = 0x490D;
static constexpr uint8_t* ADDR_DRAW_ARG3 = 0x490E;
static constexpr uint8_t* ADDR_DRAW_ARG4 = 0x490F;
static constexpr uint8_t* ADDR_TIMER1_LO = 0x4910;
static constexpr uint8_t* ADDR_TIMER1_HI = 0x4911;
static constexpr uint8_t* ADDR_TIMER2_LO = 0x4912;
static constexpr uint8_t* ADDR_TIMER2_HI = 0x4913;
static constexpr uint8_t* ADDR_COUNTER1 = 0x4914;
static constexpr uint8_t* ADDR_COUNTER2 = 0x4915;

/* Event types */
static constexpr uint8_t EVENT_UNSPECIFIED = 0x00;
static constexpr uint8_t EVENT_KEYDOWN = 0x01;
static constexpr uint8_t EVENT_KEYUP = 0x02;
static constexpr uint8_t EVENT_MOUSEMOVE = 0x03;
static constexpr uint8_t EVENT_MOUSEDOWN = 0x04;
static constexpr uint8_t EVENT_MOUSEUP = 0x05;
static constexpr uint8_t EVENT_CLOCK1 = 0x06;
static constexpr uint8_t EVENT_CLOCK2 = 0x07;
static constexpr uint8_t EVENT_TIMER1 = 0x08;
static constexpr uint8_t EVENT_TIMER2 = 0x09;
static constexpr uint8_t EVENT_COUNTER1 = 0x0A;
static constexpr uint8_t EVENT_COUNTER2 = 0x0B;

/* Keycodes returned by the keyboard interrupt */
static constexpr uint8_t KEY_W = 0x16;
static constexpr uint8_t KEY_S = 0x12;
static constexpr uint8_t KEY_I = 0x08;
static constexpr uint8_t KEY_K = 0x0A;

/* Colors used for drawing on the screen */
static constexpr uint8_t BLUE = 0x03;
static constexpr uint8_t YELLOW = 0xFC;
static constexpr uint8_t WHITE = 0xFF;
static constexpr uint8_t BLACK = 0x00;

/* Constants for the IO chip drawing API */
static constexpr uint8_t DRAW_RECTANGLE = 0x00;
static constexpr uint8_t DRAW_SQUARE = 0x01;
static constexpr uint8_t DRAW_DOT = 0x02;
static constexpr uint8_t DRAW_LINE = 0x03;
static constexpr uint8_t BRUSH_SET_BODY = 0x80;
static constexpr uint8_t BRUSH_SET_OUTLINE = 0x81;

/* Constants for the game */
static constexpr uint8_t RACKET_WIDTH = 1;
static constexpr uint8_t RACKET_HEIGHT = 8;
static constexpr uint8_t RACKET1_X = 1;
static constexpr uint8_t RACKET2_X = 62;
static constexpr uint8_t BALL_DIAMETER = 1;
static constexpr uint8_t SCREEN_HEIGHT = 36;
static constexpr uint8_t SCREEN_WIDTH = 64;
static constexpr uint8_t RACKET_INITIAL_Y = 14;
static constexpr uint8_t BALL_INITIAL_X = 18;
static constexpr uint8_t BALL_INITIAL_Y = 32;
static constexpr uint8_t WINNING_SCORE = 0x0A;
static constexpr uint8_t BEEP_SHORT_AUDIO = 0xD9;
static constexpr uint8_t BEEP_LONG_AUDIO = 0xE3;
static constexpr uint8_t SILENT_AUDIO = 0x19;

static constexpr uint8_t RACKETY_MAX = 27;
static constexpr uint8_t SCREEN_HEIGHT_M1 = 35;
static constexpr uint8_t SCREEN_HEIGHT_M2 = 34;
static constexpr uint8_t RACKET1_X_P1 = 2;
static constexpr uint8_t RACKET2_X_M1 = 61;

/* Timing & Speed control */
static constexpr uint8_t BALL_UPDATE_CLOCK = 0x10;

/* Runtime data used by the game */
/* 0x00 */ uint8_t racket1_pos;           // the y offset of the left racket
/* 0x01 */ uint8_t racket2_pos;           // the y offset of the right racket
/* 0x02 */ uint8_t score1;                // the score of the left player
/* 0x03 */ uint8_t score2;                // the score of the right player
/* 0x04 */ uint8_t ball_pos_x;            // the x position of the ball
/* 0x05 */ uint8_t ball_pos_y;            // the y position of the ball
/* 0x06 */ uint8_t ball_vel_x;            // wether the ball is moving to the right
/* 0x07 */ uint8_t ball_vel_y;            // wether the ball is moving downwards
/* 0x08 */ uint8_t int_argument_storage;  // memory to store interrupt event arguments in

/* Sets up the initial state of the game and then goes to sleep */
int main() {
  disable_interrupts();
  reset_game();
  *ADDR_CLOCK1 = BALL_UPDATE_CLOCK;
  enable_interrupts();

  // Infinite loop as we don't need this method anymore
  // This can be implemented using a WAI instruction.
  label: LOOP
    nop();
    goto LOOP;
}

/* Handle IRQ interrupts by the IO chip */
int irq() {
  switch (*ADDR_EVENT_TYPE) {
    case EVENT_KEYDOWN: {
      handle_keypress();
      break;
    }
    case EVENT_CLOCK1: {
      handle_clock();
      break;
    }
    case EVENT_TIMER1: {
      stop_beep();
      break;
    }
  }
}

void start_short_beep() {
  *ADDR_AUDIO1 = BEEP_SHORT_AUDIO;
  *ADDR_TIMER1_HI = 0x00;
  *ADDR_TIMER1_LO = 0x0A; // 0x0A -> 10 -> 100ms
}

void start_long_beep() {
  *ADDR_AUDIO1 = BEEP_LONG_AUDIO;
  *ADDR_TIMER1_HI = 0x00;
  *ADDR_TIMER1_LO = 0x1E; // 0x1E -> 30 -> 300ms
}

void stop_beep() {
  *ADDR_AUDIO1 = SILENT_AUDIO;
}

/* Resets the game after a player gained a point */
void reset_game() {
  score1 = 0;
  score2 = 0;
  reset_positions();
}

/* Handles a keypress interrupt event */
void handle_keypress() {
  int_argument_storage = *ADDR_EVENT_ARG1;

  switch (int_argument_storage) {
    case KEY_W: {
      if (racket1_pos == 0)
        break;
      racket1_pos--;
      break;
    }
    case KEY_S: {
      if (racket1_pos == SCREEN_HEIGHT - RACKET_HEIGHT)
        break;
      racket1_pos++;
      break;
    }
    case KEY_I: {
      if (racket2_pos == 0)
        break;
      racket2_pos--;
      break;
    }
    case KEY_K: {
      if (racket2_pos == SCREEN_HEIGHT - RACKET_HEIGHT)
        break;
      racket2_pos++;
      break;
    }
  }
}

/* Handles a clock interrupt event */
void handle_clock() {
  // Game logic
  check_score();
  update_ball();
  check_ball_collisions();

  // Clear screen
  set_color_black();
  clear_screen();

  // Render game elements
  set_color_white();
  draw_rackets();
  draw_ball();
  draw_score();
}

void reset_positions() {
  racket1_pos = RACKET_INITIAL_Y;
  racket2_pos = RACKET_INITIAL_Y;
  reset_ball_position();
}

void reset_ball_position() {
  ball_pos_x = BALL_INITIAL_X;
  ball_pos_y = BALL_INITIAL_Y;
  ball_vel_x = true;
  ball_vel_y = true;
}

void update_ball() {
  // Move into the x direction
  if (ball_vel_x) {
    ball_pos_x++;
  } else {
    ball_pos_x--;
  }

  // Move into the y direction
  if (ball_vel_y) {
    ball_pos_y++;
  } else {
    ball_pos_y--;
  }
}

void check_ball_collisions() {
  // Check if we hit the top or bottom walls
  if (ball_pos_y == 0)
    ball_vel_y = true;
  if (ball_pos_y == SCREEN_HEIGHT - 1)
    ball_vel_y = false;

  // Check if we hit the left racket
  if (ball_pos_y >= racket1_pos && ball_pos_y < racket1_pos + RACKET_HEIGHT) {
    if (ball_pos_x == RACKET1_X + 1) {
      ball_vel_x = true;
      start_short_beep();
    }
  }

  // Check if we hit the right racket
  if (ball_pos_y >= racket2_pos && ball_pos_y < racket2_pos + RACKET_HEIGHT) {
    if (ball_pos_x == RACKET2_X - 1) {
      ball_vel_x = false;
      start_short_beep();
    }
  }

  // Check if we hit the goal of either side
  if (ball_pos_x == 0) {
    score2++;
    reset_ball_position();
    start_long_beep();
  } else if (ball_pos_x == SCREEN_WIDTH - 1) {
    score1++;
    reset_ball_position();
    start_long_beep();
  }
}

/* Check if a player won the game */
void check_score() {
  if (score1 == WINNING_SCORE) {
    reset_game();
  }
  if (score2 == WINNING_SCORE) {
    reset_game();
  }
}

/* Set the brush color to black */
void set_color_black() {
  *ADDR_DRAW_ARG1 = BLACK;
  *ADDR_DRAW_METHOD = BRUSH_SET_BODY;
  *ADDR_DRAW_METHOD = BRUSH_SET_OUTLINE;
}

/* Set the brush color to white */
void set_color_white() {
  *ADDR_DRAW_ARG1 = WHITE;
  *ADDR_DRAW_METHOD = BRUSH_SET_BODY;
  *ADDR_DRAW_METHOD = BRUSH_SET_OUTLINE;
}

/* Set the brush color to white */
void set_color_blue() {
  *ADDR_DRAW_ARG1 = BLUE;
  *ADDR_DRAW_METHOD = BRUSH_SET_BODY;
  *ADDR_DRAW_METHOD = BRUSH_SET_OUTLINE;
}

/* Set the brush color to white */
void set_color_yellow() {
  *ADDR_DRAW_ARG1 = YELLOW;
  *ADDR_DRAW_METHOD = BRUSH_SET_BODY;
  *ADDR_DRAW_METHOD = BRUSH_SET_OUTLINE;
}

/* Draws a black rectangle over the whole screen */
void clear_screen() {
  *ADDR_DRAW_ARG1 = 0;
  *ADDR_DRAW_ARG2 = 0;
  *ADDR_DRAW_ARG3 = SCREEN_WIDTH;
  *ADDR_DRAW_ARG4 = SCREEN_HEIGHT;
  *ADDR_DRAW_METHOD = DRAW_RECTANGLE;
}

/* Draw the rackets on the screen */
void draw_rackets() {
  // Draw racket #1
  *ADDR_DRAW_ARG1 = RACKET1_X;
  *ADDR_DRAW_ARG2 = racket1_pos;
  *ADDR_DRAW_ARG3 = RACKET1_X;
  *ADDR_DRAW_ARG4 = racket1_pos + RACKET_HEIGHT;
  *ADDR_DRAW_METHOD = DRAW_LINE;

  // Draw racket #2
  *ADDR_DRAW_ARG1 = RACKET2_X;
  *ADDR_DRAW_ARG2 = racket2_pos;
  *ADDR_DRAW_ARG3 = RACKET2_X;
  *ADDR_DRAW_ARG4 = racket2_pos + RACKET_HEIGHT;
  *ADDR_DRAW_METHOD = DRAW_LINE;
}

void draw_ball() {
  *ADDR_DRAW_ARG1 = ball_pos_x;
  *ADDR_DRAW_ARG2 = ball_pos_y;
  *ADDR_DRAW_METHOD = DRAW_DOT;
}

void draw_score() {
  // Draw score #1
  set_color_yellow();
  *ADDR_DRAW_ARG1 = 0;
  *ADDR_DRAW_ARG2 = SCREEN_HEIGHT - 1;
  *ADDR_DRAW_ARG3 = score1;
  *ADDR_DRAW_ARG4 = SCREEN_HEIGHT - 1;
  *ADDR_DRAW_METHOD = DRAW_LINE;

  // Draw score #2
  set_color_blue();
  *ADDR_DRAW_ARG1 = SCREEN_WIDTH - 1;
  *ADDR_DRAW_ARG2 = SCREEN_HEIGHT - 1;
  *ADDR_DRAW_ARG3 = SCREEN_WIDTH - 1 - score1;
  *ADDR_DRAW_ARG4 = SCREEN_HEIGHT - 1;
  *ADDR_DRAW_METHOD = DRAW_LINE;

  // Draw a vertical bar at index 10
  set_color_white();
  *ADDR_DRAW_ARG1 = 10;
  *ADDR_DRAW_ARG2 = SCREEN_HEIGHT - 2;
  *ADDR_DRAW_METHOD = DRAW_DOT;

  *ADDR_DRAW_ARG1 = 10;
  *ADDR_DRAW_ARG2 = SCREEN_HEIGHT - 1;
  *ADDR_DRAW_METHOD = DRAW_DOT;
}
