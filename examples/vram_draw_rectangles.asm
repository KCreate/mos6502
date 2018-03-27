; this example program draws two rectangles on the screen,
; one colored red and the other colored green

; some commonly used addresses
.def ADDR_VRAM 0x400
.def ADDR_DRAW_METHOD 0x90B
.def ADDR_DRAW_ARG1 0x90C
.def ADDR_DRAW_ARG2 0x90D
.def ADDR_DRAW_ARG3 0x90E
.def ADDR_DRAW_ARG4 0x90F

; method code of the draw rectangle IOChip intrinsic
.def DRAW_RECTANGLE 0x00
.def BRUSH_SET_BODY 0x03
.def BRUSH_SET_OUTLINE 0x04

; color codes
.def RED #$0xE0
.def GREEN #$0x1C

; load at reset entry
.RST

  ; set the brush color to red
  LDA RED
  STA ADDR_DRAW_ARG1
  LDA BRUSH_SET_BODY
  STA ADDR_DRAW_METHOD

  ; draw the rectangle
  LDA #$10              ; x coordinate
  STA ADDR_DRAW_ARG1
  LDA #$08              ; y coordinate
  STA ADDR_DRAW_ARG2
  LDA #$10              ; width
  STA ADDR_DRAW_ARG3
  LDA #$26              ; height
  STA ADDR_DRAW_ARG4
  LDA DRAW_RECTANGLE
  STA ADDR_DRAW_METHOD

  ; set the brush color to green
  LDA GREEN
  STA ADDR_DRAW_ARG1
  LDA BRUSH_SET_BODY
  STA ADDR_DRAW_METHOD

  ; draw the rectangle
  LDA #$20              ; x coordinate
  STA ADDR_DRAW_ARG1
  LDA #$08              ; y coordinate
  STA ADDR_DRAW_ARG2
  LDA #$10              ; width
  STA ADDR_DRAW_ARG3
  LDA #$26              ; height
  STA ADDR_DRAW_ARG4
  LDA DRAW_RECTANGLE
  STA ADDR_DRAW_METHOD

.END
  NOP
  JMP .END
