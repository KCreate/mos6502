; this example program draws a rectangle on the screen
; pressing the a key on the keyboard will move the rectangle to the left
; pressing the b key on the keyboard will move the rectangle to the right

; some commonly used addresses
.def ADDR_VRAM 0x4000
.def ADDR_EVENT_TYPE 0x4903
.def ADDR_KEYBOARD_KEYCODE 0x4904
.def ADDR_DRAW_METHOD 0x490B
.def ADDR_DRAW_ARG1 0x490C
.def ADDR_DRAW_ARG2 0x490D
.def ADDR_DRAW_ARG3 0x490E
.def ADDR_DRAW_ARG4 0x490F

; method code of the draw rectangle IOChip intrinsic
.def DRAW_RECTANGLE 0x00
.def BRUSH_SET_BODY 0x03
.def BRUSH_SET_OUTLINE 0x04

; color codes
.def RED #$E0
.def BLUE #$03

; event types
.def EVENT_KEYDOWN 0x01

; keyboard keycodes
.def KEY_A 0x00
.def KEY_D 0x03

; load at reset entry
.RST

  ; set the brush color to blue
  lda RED
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_BODY
  sta ADDR_DRAW_METHOD
  lda BRUSH_SET_OUTLINE
  sta_ADDR_DRAW_METHOD

; Drawing loop
.DRAW
  stx ADDR_DRAW_ARG1    ; x coordinate
  lda #$10              ; y coordinate
  sta ADDR_DRAW_ARG2
  lda #$10              ; width / height
  sta ADDR_DRAW_ARG3
  sta ADDR_DRAW_ARG4
  lda DRAW_RECTANGLE
  sta ADDR_DRAW_METHOD
  jmp .DRAW

; increase the X register each time the clock pulses
; keycode A: 0x00
; keycode D: 0x03
.IRQ

  ; Backup the X register
  txa
  pha

  ; Check if this is a keyboard event
  ldx ADDR_EVENT_TYPE
  cpx EVENT_KEYDOWN
  bne .RESTORE_AND_EXIT

  ; Check if either the A or D key were pressed
  ldx ADDR_KEYBOARD_KEYCODE
  cpx KEY_A
  beq .HANDLE_A_KEY
  cpx KEY_B
  beq .HANDLE_B_KEY

.RESTORE_AND_EXIT
  pla
  tax
  rti

.HANDLE_A_KEY
  pla
  tax
  dex
  rti

.HANDLE_B_KEY
  pla
  tax
  inx
  rti
