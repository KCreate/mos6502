// RST
    0x78
    0x20 0x79 0x49
    0xa9 0x10
    0x8d 0x06 0x49
    0x58

// L_MAIN_LOOP
    0xea
    0x4c 0x2a 0x49

// L_IRQ
    ad 03 49
    c9 01
    f0 0b
    c9 06
    f0 0d
    c9 08
    f0 0f
    4c 52 49

// L_IRQ_KEYDOWM
    20 83 49
    4c 52 49

// L_IRQ_CLOCK
    20 d4 49
    4c 52 49

// L_IRQ_TIMER
    20 73 49
    4c 52 49

// L_IRQ_EXIT
    40

// L_START_SHORT_BEEP
    a9 d9
    8d 08 49
    a9 00
    8d 11 49
    a9 0a
    8d 10 49
    60

// L_START_LONG_BEEP
    a9 d9
    8d 08 49
    a9 00
    8d 11 49
    a9 1e
    8d 10 49
    60

// L_STOP_BEEP
    a9 19
    8d 08 49
    60

// L_RESET_GAME
    a9 00
    85 02
    85 03
    20 f0 49
    60

// L_HANDLE_KEYPRESS
    ad 04 49
    85 08
    c9 16
    f0 0f
    c9 12
    f0 19
    c9 08
    f0 23
    c9 0a
    f0 2d
    4c d3 49

// L_HANDLE_KEYPRESS_W
    a5 00
    c9 00
    f0 32
    18
    e9 01
    85 00
    4c d3 49

// L_HANDLE_KEYPRESS_S
    a5 00
    c9 1b
    f0 24
    18
    69 01
    85 00
    4c d3 49

// L_HANDLE_KEYPRESS_I
    a5 01
    c9 00
    f0 16
    18
    e9 01
    85 01
    4c d3 49

// L_HANDLE_KEYPRESS_K
    a5 01
    c9 1b
    f0 08
    18
    69 01
    85 01
    4c d3 49

// L_HANDLE_KEYPRESS_EXIT
    60

// L_HANDLE_CLOCK
    20 a4 4a
    20 ff 49
    20 31 4a
    20 b4 4a
    20 d4 4a
    20 c4 4a
    20 ec 4a
    20 1d 4b
    20 2d 4b
    60

// L_RESET_POSITIONS
    a9 20
    85 04
    85 05
    a9 01
    85 06
    a9 01
    85 07
    60

// L_UPDATE_BALL
    a5 06
    d0 0a
    a5 04
    18
    e9 01
    85 04
    4c 17 4a

// L_UPDATE_BALL_X_INC
    a5 04
    18
    69 01
    85 04
    4c 17 4a

// L_UPDATE_BALL_X_END
    a5 07
    d0 0a
    a5 05
    18
    e9 01
    85 05
    4c 30 4a

// L_UPDATE_BALL_Y_INC
    a5 05
    18
    69 01
    85 05
    ea
    4c 30 4a

// L_UPDATE_BALL_Y_END
    60

// L_CHECK_BALL_COLLISIONS
    a5 05
    c9 00
    d0 07
    a9 01
    85 07
    20 53 49

// L_CHECK_BALL_COLLISIONS_NO_TOP_HIT
    a5 05
    c9 23
    d0 07
    a9 00
    85 07
    20 53 49

// L_CHECK_BALL_COLLISIONS_NO_BOTTOM_HIT
    a5 05
    c5 00
    b0 16
    a5 05
    18
    e5 00
    c9 08
    90 0d
    a5 04
    c9 02
    d0 07
    a9 01
    85 06
    20 53 49

// L_CHECK_BALL_COLLISIONS_NO_LEFT_HIT
    a5 05
    c5 01
    b0 16
    a5 05
    18
    e5 01
    c9 08
    90 0d
    a5 04
    c9 02
    d0 07
    a9 00
    85 06
    20 53 49

// L_CHECK_BALL_COLLISIONS_NO_RIGHT_HIT
    a5 04
    c9 00
    d0 0d
    a5 03
    18
    69 01
    85 03
    20 5f 4b
    20 63 49

// L_CHECK_BALL_COLLISIONS_INC_1_SCORE
    a5 02
    18
    69 01
    85 02
    20 5f 4b
    20 63 49

// L_CHECK_BALL_COLLISIONS_EXIT
    60

// L_CHECK_SCORE
    a5 02
    c9 0a
    f0 06
    a5 03
    c9 0a
    f0 00

// L_CHECK_SCORE_RESET
    20 79 49

// L_CHECK_SCORE_END
    60

// L_SET_COLOR_BLACK
    a9 00
    8d 0c 49
    a9 80
    8d 0b 49
    a9 81
    8d 0b 49
    60

// L_SET_COLOR_WHITE
    a9 ff
    8d 0c 49
    a9 80
    8d 0b 49
    a9 81
    8d 0b 49
    60

// L_CLEAR_SCREEN
    a9 00
    8d 0c 49
    8d 0d 49
    a9 40
    8d 0e 49
    a9 24
    8d 0f 49
    a9 00
    8d 0b 49
    60

// L_DRAW_RACKETS
    a9 01
    8d 0c 49
    8d 0e 49
    a5 00
    8d 0d 49
    18
    69 08
    8d 0f 49
    a9 03
    8d 0b 49
    a9 3e
    8d 0c 49
    8d 0e 49
    a5 01
    8d 0d 49
    18
    69 08
    8d 0f 49
    a9 03
    8d 0b 49
    60

// L_DRAW_BALL
    a5 04
    8d 0c 49
    a5 05
    8d 0d 49
    a9 02
    8d 0b 49
    60

// L_DRAW_SCORE
    a9 00
    8d 0c 49
    a9 23
    8d 0d 49
    a5 02
    8d 0e 49
    a9 23
    8d 0f 49
    a9 03
    8d 0b 49
    a9 00
    8d 0c 49
    a9 22
    8d 0d 49
    a5 03
    8d 0e 49
    a9 22
    8d 0f 49
    a9 03
    8d 0b 49
    60

// L_RESET_BALL_POSITION
    a9 20
    85 04
    85 05
    a9 01
    85 06
    a9 01
    85 07
    60
