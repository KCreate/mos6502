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
  LDA #$C8
  STA ADDR_CLOCK1

  ; set the brush color to red
  LDA RED
  STA ADDR_DRAW_ARG1
  LDA BRUSH_SET_BODY
  STA ADDR_DRAW_METHOD

  ; set the outline color to blue
  LDA BLUE
  STA ADDR_DRAW_ARG1
  LDA BRUSH_SET_BODY
  STA_ADDR_DRAW_METHOD

; Draw the rectangle
.DRAW
  STX ADDR_DRAW_ARG1    ; x coordinate
  LDA #$08              ; y coordinate
  STA ADDR_DRAW_ARG2
  LDA #$10              ; width
  STA ADDR_DRAW_ARG3
  LDA #$14              ; height
  STA ADDR_DRAW_ARG4
  LDA DRAW_RECTANGLE
  STA ADDR_DRAW_METHOD
  JMP .DRAW

; increase the X register each time the clock pulses
.IRQ
  INX
  RTI
