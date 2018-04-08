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
.def BRUSH_SET_BODY 0x80
.def BRUSH_SET_OUTLINE 0x81

; color codes
.def RED #$E0
.def BLUE #$03

; event types
.def EVENT_KEYDOWN 0x01

; keyboard keycodes
.def KEY_W 0x16
.def KEY_A 0x00
.def KEY_S 0x12
.def KEY_D 0x03

; load at reset entry
.RST

  ; set the body color to red
  lda RED
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_BODY
  sta ADDR_DRAW_METHOD

  ; set the outline color to blue
  lda BLUE
  sta ADDR_DRAW_ARG1
  lda BRUSH_SET_OUTLINE
  sta_ADDR_DRAW_METHOD

; Drawing loop
.DRAW
  lda #$10              ; width / height
  sta ADDR_DRAW_ARG3
  sta ADDR_DRAW_ARG4
  lda DRAW_RECTANGLE
.DRAWLOOP
  stx ADDR_DRAW_ARG1    ; x coordinate
  sty ADDR_DRAW_ARG2    ; y coordinate
  sta ADDR_DRAW_METHOD
  jmp .DRAWLOOP

; Move the rectangle with the WASD keys
.IRQ

  ; Backup A register
  PHA

  ; Check if this is a keyboard event
  lda ADDR_EVENT_TYPE
  cmp EVENT_KEYDOWN
  bne .RESTORE_AND_EXIT

  ; Check if either the A or D key were pressed
  lda ADDR_KEYBOARD_KEYCODE
  cmp KEY_W
  beq .HANDLE_W_KEY
  cmp KEY_A
  beq .HANDLE_A_KEY
  cmp KEY_S
  beq .HANDLE_D_KEY
  cmp KEY_D
  beq .HANDLE_D_KEY

.RESTORE_AND_EXIT
  PLA
  rti

.HANDLE_W_KEY
  PLA
  dey
  rti

.HANDLE_A_KEY
  PLA
  dex
  rti

.HANDLE_S_KEY
  PLA
  iny
  rti

.HANDLE_D_KEY
  PLA
  inx
  rti
