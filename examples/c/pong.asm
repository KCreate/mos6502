; Load the program at 0x4920
* = $4920

; Adresses on the machine
define ADDR_CONTROL $4900
define ADDR_TEXT_MODE_BACKGROUND_COLOR $4901
define ADDR_TEXT_MODE_FOREGROUND_COLOR $4902
define ADDR_EVENT_TYPE $4903
define ADDR_EVENT_ARG1 $4904
define ADDR_EVENT_ARG2 $4905
define ADDR_CLOCK1 $4906
define ADDR_CLOCK2 $4907
define ADDR_AUDIO1 $4908
define ADDR_AUDIO2 $4909
define ADDR_AUDIO3 $490A
define ADDR_DRAW_METHOD $490B
define ADDR_DRAW_ARG1 $490C
define ADDR_DRAW_ARG2 $490D
define ADDR_DRAW_ARG3 $490E
define ADDR_DRAW_ARG4 $490F
define ADDR_TIMER1_LO $4910
define ADDR_TIMER1_HI $4911
define ADDR_TIMER2_LO $4912
define ADDR_TIMER2_HI $4913
define ADDR_COUNTER1 $4914
define ADDR_COUNTER2 $4915

; Various constants used when interacting with the machine
define EVENT_UNSPECIFIED $00
define EVENT_KEYDOWN $01
define EVENT_KEYUP $02
define EVENT_MOUSEMOVE $03
define EVENT_MOUSEDOWN $04
define EVENT_MOUSEUP $05
define EVENT_CLOCK1 $06
define EVENT_CLOCK2 $07
define EVENT_TIMER1 $08
define EVENT_TIMER2 $09
define EVENT_COUNTER1 $0A
define EVENT_COUNTER2 $0B
define KEY_W $16
define KEY_S $12
define KEY_I $08
define KEY_K $0A
define BLUE $03
define YELLOW $FC
define WHITE $FF
define BLACK $00
define DRAW_RECTANGLE $00
define DRAW_SQUARE $01
define DRAW_DOT $02
define DRAW_LINE $03
define BRUSH_SET_BODY $80
define BRUSH_SET_OUTLINE $81
define RACKET_WIDTH 1
define RACKET_HEIGHT 8
define RACKET1_X 1
define RACKET2_X 62
define BALL_DIAMETER 1
define SCREEN_HEIGHT 36
define SCREEN_WIDTH 64
define RACKET_INITIAL_Y 14
define BALL_INITIAL_X 18
define BALL_INITIAL_Y 32
define WINNING_SCORE 10
define BEEP_SHORT_AUDIO $D9
define BEEP_LONG_AUDIO $D3
define SILENT_AUDIO $19
define RACKETY_MAX 28
define SCREEN_HEIGHT_M1 35
define SCREEN_HEIGHT_M2 34
define SCREEN_WIDTH_M1 63
define RACKET1_X_P1 2
define RACKET2_X_M1 61
define BALL_UPDATE_CLOCK $10

; Memory locations storing program variables
define V_RACKET1_POS $00
define V_RACKET2_POS $01
define V_SCORE1 $02
define V_SCORE2 $03
define V_BALL_POS_X $04
define V_BALL_POS_Y $05
define V_BALL_VEL_X $06
define V_BALL_VEL_Y $07
define V_INT_ARGUMENT_STORAGE $08

L_RST:
   sei
   jsr L_RESET_GAME
   lda #BALL_UPDATE_CLOCK
   sta ADDR_CLOCK1
   cli

L_MAIN_LOOP:
   wai
   jmp L_MAIN_LOOP

L_IRQ:
   lda ADDR_EVENT_TYPE
   cmp #EVENT_KEYDOWN
   beq L_IRQ_KEYDOWN
   cmp #EVENT_CLOCK1
   beq L_IRQ_CLOCK
   cmp #EVENT_TIMER1
   beq L_IRQ_TIMER
   jmp L_IRQ_EXIT

L_IRQ_KEYDOWN:
   jsr L_HANDLE_KEYPRESS
   jmp L_IRQ_EXIT

L_IRQ_CLOCK:
   jsr L_HANDLE_CLOCK
   jmp L_IRQ_EXIT

L_IRQ_TIMER:
   jsr L_STOP_BEEP
   jmp L_IRQ_EXIT

L_IRQ_EXIT:
   rti

L_START_SHORT_BEEP:
   lda #BEEP_SHORT_AUDIO
   sta ADDR_AUDIO1
   lda #$00
   sta ADDR_TIMER1_HI
   lda #$0A
   sta ADDR_TIMER1_LO
   rts

L_START_LONG_BEEP:
   lda #BEEP_LONG_AUDIO
   sta ADDR_AUDIO1
   lda #$00
   sta ADDR_TIMER1_HI
   lda #$1E
   sta ADDR_TIMER1_LO
   rts

L_STOP_BEEP:
   lda #SILENT_AUDIO
   sta ADDR_AUDIO1
   rts

L_RESET_GAME:
   lda #$00
   sta V_SCORE1
   lda #$00
   sta V_SCORE2
   jsr L_RESET_POSITIONS
   rts

L_HANDLE_KEYPRESS:
   lda ADDR_EVENT_ARG1
   sta V_INT_ARGUMENT_STORAGE
   cmp #KEY_W
   beq L_HANDLE_KEYPRESS_W
   cmp #KEY_S
   beq L_HANDLE_KEYPRESS_S
   cmp #KEY_I
   beq L_HANDLE_KEYPRESS_I
   cmp #KEY_K
   beq L_HANDLE_KEYPRESS_K
   jmp L_HANDLE_KEYPRESS_EXIT

L_HANDLE_KEYPRESS_W:
   lda V_RACKET1_POS
   cmp #$00
   beq L_HANDLE_KEYPRESS_EXIT
   clc
   sbc #$01
   sta V_RACKET1_POS
   jmp L_HANDLE_KEYPRESS_EXIT

L_HANDLE_KEYPRESS_S:
   lda V_RACKET1_POS
   cmp #RACKETY_MAX
   beq L_HANDLE_KEYPRESS_EXIT
   clc
   adc #$01
   sta V_RACKET1_POS
   jmp L_HANDLE_KEYPRESS_EXIT

L_HANDLE_KEYPRESS_I:
   lda V_RACKET2_POS
   cmp #$00
   beq L_HANDLE_KEYPRESS_EXIT
   clc
   sbc #$01
   sta V_RACKET2_POS
   jmp L_HANDLE_KEYPRESS_EXIT

L_HANDLE_KEYPRESS_K:
   lda V_RACKET2_POS
   cmp #RACKETY_MAX
   beq L_HANDLE_KEYPRESS_EXIT
   clc
   adc #$01
   sta V_RACKET2_POS
   jmp L_HANDLE_KEYPRESS_EXIT

L_HANDLE_KEYPRESS_EXIT:
   rts

L_HANDLE_CLOCK:
   jsr L_CHECK_SCORE
   jsr L_UPDATE_BALL
   jsr L_CHECK_BALL_COLLISIONS

   jsr L_SET_COLOR_BLACK
   jsr L_CLEAR_SCREEN

   jsr L_SET_COLOR_WHITE
   jsr L_DRAW_RACKETS
   jsr L_DRAW_BALL
   jsr L_DRAW_SCORE
   rts

L_RESET_POSITIONS:
   lda #RACKET_INITIAL_Y
   sta V_RACKET1_POS
   lda #RACKET_INITIAL_Y
   sta V_RACKET2_POS
   jsr L_RESET_BALL_POSITION
   rts

L_RESET_BALL_POSITION:
   lda #BALL_INITIAL_Y
   sta V_BALL_POS_X
   lda #BALL_INITIAL_X
   sta V_BALL_POS_Y
   lda #$01
   sta V_BALL_VEL_X
   lda #$01
   sta V_BALL_VEL_Y
   rts

L_UPDATE_BALL:
   lda V_BALL_VEL_X
   bne L_UPDATE_BALL_X_INC
   lda V_BALL_POS_X
   clc
   sbc #$01
   sta V_BALL_POS_X
   jmp L_UPDATE_BALL_X_END

L_UPDATE_BALL_X_INC:
   lda V_BALL_POS_X
   clc
   adc #$01
   sta V_BALL_POS_X
   jmp L_UPDATE_BALL_X_END

L_UPDATE_BALL_X_END:
   lda V_BALL_VEL_Y
   bne L_UPDATE_BALL_Y_INC
   lda V_BALL_POS_Y
   clc
   sbc #$01
   sta V_BALL_POS_Y
   jmp L_UPDATE_BALL_Y_END

L_UPDATE_BALL_Y_INC:
   lda V_BALL_POS_Y
   clc
   adc #$01
   sta V_BALL_POS_Y
   nop
   jmp L_UPDATE_BALL_Y_END

L_UPDATE_BALL_Y_END:
   rts

L_CHECK_BALL_COLLISIONS:
   lda V_BALL_POS_Y
   cmp #$00
   bne L_CHECK_BALL_COLLISIONS_NO_TOP_HIT
   lda #$01
   sta V_BALL_VEL_Y
   jsr L_START_SHORT_BEEP

L_CHECK_BALL_COLLISIONS_NO_TOP_HIT:
   lda V_BALL_POS_Y
   cmp #SCREEN_HEIGHT_M1
   bne L_CHECK_BALL_COLLISIONS_NO_BOTTOM_HIT
   lda #$00
   sta V_BALL_VEL_Y
   jsr L_START_SHORT_BEEP





L_CHECK_BALL_COLLISIONS_NO_BOTTOM_HIT:
   lda V_BALL_POS_Y
   cmp V_RACKET1_POS
   bcc L_CHECK_BALL_COLLISIONS_NO_LEFT_HIT
   lda V_BALL_POS_Y
   clc
   sbc V_RACKET1_POS
   cmp #RACKET_HEIGHT
   bcs L_CHECK_BALL_COLLISIONS_NO_LEFT_HIT
   lda V_BALL_POS_X
   cmp #RACKET1_X_P1
   bne L_CHECK_BALL_COLLISIONS_NO_LEFT_HIT
   lda #$01
   sta V_BALL_VEL_X
   jsr L_START_SHORT_BEEP

L_CHECK_BALL_COLLISIONS_NO_LEFT_HIT:
   lda V_BALL_POS_Y
   cmp V_RACKET2_POS
   bcc L_CHECK_BALL_COLLISIONS_NO_RIGHT_HIT
   lda V_BALL_POS_Y
   clc
   sbc V_RACKET2_POS
   cmp #RACKET_HEIGHT
   bcs L_CHECK_BALL_COLLISIONS_NO_RIGHT_HIT
   lda V_BALL_POS_X
   cmp #RACKET2_X_M1
   bne L_CHECK_BALL_COLLISIONS_NO_RIGHT_HIT
   lda #$00
   sta V_BALL_VEL_X
   jsr L_START_SHORT_BEEP







L_CHECK_BALL_COLLISIONS_NO_RIGHT_HIT:
   lda V_BALL_POS_X
   cmp #$00
   bne L_CHECK_BALL_COLLISIONS_CHECK_RIGHT_GOAL
   lda V_SCORE2
   clc
   adc #$01
   sta V_SCORE2
   jsr L_RESET_BALL_POSITION
   jsr L_START_LONG_BEEP

L_CHECK_BALL_COLLISIONS_CHECK_RIGHT_GOAL:

   lda V_BALL_POS_X
   cmp #SCREEN_WIDTH_M1
   bne L_CHECK_BALL_COLLISIONS_EXIT

   lda V_SCORE1
   clc
   adc #$01
   sta V_SCORE1
   jsr L_RESET_BALL_POSITION
   jsr L_START_LONG_BEEP

L_CHECK_BALL_COLLISIONS_EXIT:
   rts

L_CHECK_SCORE:
   lda V_SCORE1
   cmp #$0A
   beq L_CHECK_SCORE_RESET
   lda V_SCORE2
   cmp #$0A
   beq L_CHECK_SCORE_RESET
   jmp L_CHECK_SCORE_END

L_CHECK_SCORE_RESET:
   jsr L_RESET_GAME

L_CHECK_SCORE_END:
   rts

L_SET_COLOR_BLACK:
   lda #BLACK
   sta ADDR_DRAW_ARG1
   lda #BRUSH_SET_BODY
   sta ADDR_DRAW_METHOD
   lda #BRUSH_SET_OUTLINE
   sta ADDR_DRAW_METHOD
   rts

L_SET_COLOR_WHITE:
   lda #WHITE
   sta ADDR_DRAW_ARG1
   lda #BRUSH_SET_BODY
   sta ADDR_DRAW_METHOD
   lda #BRUSH_SET_OUTLINE
   sta ADDR_DRAW_METHOD
   rts

L_SET_COLOR_BLUE:
   lda #BLUE
   sta ADDR_DRAW_ARG1
   lda #BRUSH_SET_BODY
   sta ADDR_DRAW_METHOD
   lda #BRUSH_SET_OUTLINE
   sta ADDR_DRAW_METHOD
   rts

L_SET_COLOR_YELLOW:
   lda #YELLOW
   sta ADDR_DRAW_ARG1
   lda #BRUSH_SET_BODY
   sta ADDR_DRAW_METHOD
   lda #BRUSH_SET_OUTLINE
   sta ADDR_DRAW_METHOD
   rts

L_CLEAR_SCREEN:
   lda #$00
   sta ADDR_DRAW_ARG1
   sta ADDR_DRAW_ARG2
   lda #SCREEN_WIDTH
   sta ADDR_DRAW_ARG3
   lda #SCREEN_HEIGHT
   sta ADDR_DRAW_ARG4
   lda #DRAW_RECTANGLE
   sta ADDR_DRAW_METHOD
   rts

L_DRAW_RACKETS:
   lda #RACKET1_X
   sta ADDR_DRAW_ARG1
   sta ADDR_DRAW_ARG3
   lda V_RACKET1_POS
   sta ADDR_DRAW_ARG2
   clc
   adc #RACKET_HEIGHT
   sta ADDR_DRAW_ARG4
   lda #DRAW_LINE
   sta ADDR_DRAW_METHOD

   lda #RACKET2_X
   sta ADDR_DRAW_ARG1
   sta ADDR_DRAW_ARG3
   lda V_RACKET2_POS
   sta ADDR_DRAW_ARG2
   clc
   adc #RACKET_HEIGHT
   sta ADDR_DRAW_ARG4
   lda #DRAW_LINE
   sta ADDR_DRAW_METHOD
   rts

L_DRAW_BALL:
   lda V_BALL_POS_X
   sta ADDR_DRAW_ARG1
   lda V_BALL_POS_Y
   sta ADDR_DRAW_ARG2
   lda #DRAW_DOT
   sta ADDR_DRAW_METHOD
   rts

L_DRAW_SCORE:

   jsr L_SET_COLOR_YELLOW

   lda #$00
   sta ADDR_DRAW_ARG1
   lda #SCREEN_HEIGHT_M1
   sta ADDR_DRAW_ARG2
   lda V_SCORE1
   sta ADDR_DRAW_ARG3
   lda #SCREEN_HEIGHT_M1
   sta ADDR_DRAW_ARG4
   lda #DRAW_LINE
   sta ADDR_DRAW_METHOD

   jsr L_SET_COLOR_BLUE

   lda #$00
   sta ADDR_DRAW_ARG1
   lda #SCREEN_HEIGHT_M2
   sta ADDR_DRAW_ARG2
   lda V_SCORE2
   sta ADDR_DRAW_ARG3
   lda #SCREEN_HEIGHT_M2
   sta ADDR_DRAW_ARG4
   lda #DRAW_LINE
   sta ADDR_DRAW_METHOD

   jsr L_SET_COLOR_WHITE

   lda #$0A
   sta ADDR_DRAW_ARG1
   lda #SCREEN_HEIGHT_M2
   sta ADDR_DRAW_ARG2
   lda #DRAW_DOT
   sta ADDR_DRAW_METHOD
   lda #$0A
   sta ADDR_DRAW_ARG1
   lda #SCREEN_HEIGHT_M1
   sta ADDR_DRAW_ARG2
   lda #DRAW_DOT
   sta ADDR_DRAW_METHOD

   rts