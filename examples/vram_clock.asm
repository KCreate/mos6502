; this example program draws a rectangle on the screen
; it configures clock1 to pulse every second, moving the
; rectangle one position to the right

; some commonly used addresses
.def ADDR_VRAM 0x400
.def ADDR_CLOCK1 0x906
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
.def RED #$E0
.def BLUE #$03

; load at reset entry
.RST

  ; setup the clock to pulse every 1000 milliseconds
  lda #$C8
  sta ADDR_CLOCK1

  ; set the brush color to red
  lda RED
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_BODY
  sta ADDR_DRAW_METHOD

  ; set the outline color to blue
  lda BLUE
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_BODY
  sta ADDR_DRAW_METHOD

; Draw the rectangle
.DRAW
  stx ADDR_DRAW_ARG1    ; x coordinate
  lda #$08              ; y coordinate
  sta ADDR_DRAW_ARG2
  lda #$10              ; width
  sta ADDR_DRAW_ARG3
  lda #$14              ; height
  sta ADDR_DRAW_ARG4
  lda DRAW_RECTANGLE
  sta ADDR_DRAW_METHOD
  jmp .DRAW

; increase the X register each time the clock pulses
.IRQ
  inx
  rti
