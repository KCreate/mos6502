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
  lda RED
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_BODY
  sta ADDR_DRAW_METHOD

  ; draw the rectangle
  lda #$10              ; x coordinate
  sta ADDR_DRAW_ARG1
  lda #$08              ; y coordinate
  sta ADDR_DRAW_ARG2
  lda #$10              ; width
  sta ADDR_DRAW_ARG3
  lda #$26              ; height
  sta ADDR_DRAW_ARG4
  lda DRAW_RECTANGLE
  sta ADDR_DRAW_METHOD

  ; set the brush color to green
  lda GREEN
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_BODY
  sta ADDR_DRAW_METHOD

  ; draw the rectangle
  lda #$20              ; x coordinate
  sta ADDR_DRAW_ARG1
  lda #$08              ; y coordinate
  sta ADDR_DRAW_ARG2
  lda #$10              ; width
  sta ADDR_DRAW_ARG3
  lda #$26              ; height
  sta ADDR_DRAW_ARG4
  lda DRAW_RECTANGLE
  sta ADDR_DRAW_METHOD

.END
  nop
  jmp .END
