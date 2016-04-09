; *****************************************************************************
;                           SPRITE LIBRARY
; *****************************************************************************



.ramsection "Spritelib Variables" slot 3
  SpriteBufferY dsb 64
  SpriteBufferXC dsb 128
  NextFreeSprite db
  MasterY db
  MasterX db
.ends
; -----------------------------------------------------------------------------
.section "LoadSAT" free
; -----------------------------------------------------------------------------
  ; Load the vram sat with the SatY and SatXC buffers
  ; Time: 15 lines, 64 sprites are updated with x,y positions and char codes.
  ; FIXME: Add flickering a'la Sonic.
  LoadSAT:
    ; Load y-coordinates
    ld hl,SAT_Y_START
    ld a,l
    out (CONTROL_PORT),a
    ld a,h
    or VRAM_WRITE_COMMAND
    out (CONTROL_PORT),a
    ld hl,SpriteBufferY
    ld c,DATA_PORT
    .rept 64
      outi
    .endr

    ; Load x-coordinates and character codes
    ld hl,SAT_XC_START
    ld a,l
    out (CONTROL_PORT),a
    ld a,h
    or VRAM_WRITE_COMMAND
    out (CONTROL_PORT),a
    ld hl,SpriteBufferXC
    ld c,DATA_PORT
    .rept 128
      outi
    .endr
  ret
.ends

; -----------------------------------------------------------------------------
.section "MetaSprites" free
; -----------------------------------------------------------------------------
  BeginMetaSprites:
    ; Reset the NextFreeSprite index at the beginning of every frame
    ld a,FIRST_FREE_SPRITE
    ld (NextFreeSprite),a
  ret

FinalizeMetaSprites:
  ; Put the terminator in the buffer
  ld de,SpriteBufferY
  ld a,(NextFreeSprite)
  add a,e
  ld e,a
  ld a,0
  adc a,d
  ld d,a
  ld a,SPRITE_TERMINATOR
  ld (de),a
ret
.ends

.section "AddMetaSprite" free
AddMetaSprite:
; Entry: NextFreeSprite is the first buffer position.
; HL = Base address of meta sprite array. First byte is meta sprite size.
; A = Master Y
; B = Master X
; Procedure: First the program processes the Y-offsets. Each offset is
; applied to the master Y, and the result is saved in SpriteBufferY. Then the
; X-offsets are processed, and stored pairwise with char codes in SpriteBufferXC
; Exit: SpriteBufferY and SpriteBufferXC is updated. NextFreeSprite is
; incremented.
; Assume master y,x to be top left corner

  ; Save the parameters in ram
  ld (MasterY),a
  cp 169 ; just outside lcd area
  ret nc ; quick and dirty sprite clipping technique
  ld a,b
  ld (MasterX),a

  ; Point DE to SpriteBufferY[NextFreeSprite]
  ld a,(NextFreeSprite)
  ld de,SpriteBufferY
  add a,e
  ld e,a
  ld a,0
  adc a,d
  ld d,a

  ; Get size of metasprite and put it in B
  ld a,(hl)
  inc hl ; now hl points to first y-offset
  ld b,a ; put size in B, so we can use DJNZ

  ; Apply offsets to MasterY, and save the results in the SpriteBufferY
  push bc ; save the counter
  ld a,(MasterY)
  ld c,a
  -:
    ld a,(hl)
    add a,c
    ld (de),a
    inc hl
    inc de
  djnz -
  pop bc ; restore counter in B

  ; Apply offsets to MasterX, and save results, together with charcodes, in
  ; SpriteBufferXC.
  ; HL is pointing at the first x-offset. B holds the size.
  push bc ; save the counter
  ; First, point DE to SpriteBufferXC[NextFreeSprite]
  ; Point DE to SpriteBufferY[NextFreeSprite]
  ld a,(NextFreeSprite)
  add a,a ; because we are having x,charcode pairs (words) in this buffer.
  ld de,SpriteBufferXC
  add a,e
  ld e,a
  ld a,0
  adc a,d
  ld d,a ; Now, DE is pointing correctly into the SpriteBufferXC

  ld a,(MasterX)
  ld c,a
  -:
    ld a,(hl)
    add a,c
    ld (de),a
    inc hl
    inc de
    ld a,(hl) ; get the char code
    ld (de),a
    inc hl
    inc de
  djnz -

  pop bc ; restore counter
  ld a,(NextFreeSprite) ; add the size of the processed metasprite
  add a,b
  ld (NextFreeSprite),a

ret
.ends
