; *****************************************************************************
;                           SPRITE LIBRARY
; *****************************************************************************
; Note: Spritelib depends on constants from Base.inc.

.equ FIRST_FREE_SPRITE 0    ; First sprite controlled by this library.
.equ Y_MAX 193              ; For quick and dirty clipping.


.ramsection "Spritelib Variables" slot 3
  SpriteBufferY dsb 64
  SpriteBufferXC dsb 128
  NextFreeSprite db
  MasterY db
  MasterX db
  MetaSpriteSize db
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
.section "BeginMetaSprites" free
; -----------------------------------------------------------------------------
  BeginMetaSprites:
    ; Reset the NextFreeSprite index at the beginning of every frame
    ld a,FIRST_FREE_SPRITE
    ld (NextFreeSprite),a
  ret
.ends

; -----------------------------------------------------------------------------
.section "FinalizeMetaSprites" free
; -----------------------------------------------------------------------------
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

; -----------------------------------------------------------------------------
.section "AddMetaSprite" free
; -----------------------------------------------------------------------------
  AddMetaSprite:
    ; Entry: NextFreeSprite is the first buffer position.
    ;        HL = Base address of meta sprite array. First byte is the size of
    ;        the meta sprite.
    ;        A = Master Y
    ;        B = Master X
    ; Procedure: First the program processes the Y-offsets. Each offset is
    ; applied to the master Y, and the result is saved in SpriteBufferY. Then
    ; the X-offsets are processed, and stored pairwise with character codes in
    ; SpriteBufferXC.
    ; Exit: SpriteBufferY and SpriteBufferXC is updated. NextFreeSprite is
    ; incremented.

    ; Perform vertical clipping test and save the parameters in ram.
    ld (MasterY),a
    cp Y_MAX
    ret nc                ; Quick and dirty sprite clipping technique.
    ld a,b
    ld (MasterX),a

    ; Get size of metasprite and put it in B.
    ld a,(hl)             ; HL points to size on entry.
    inc hl                ; Now hl points to first y-offset.
    ld b,a                ; Put size in B, so we can use DJNZ.
    ld (MetaSpriteSize),a ; Save the size for later counting jobs.

    ; Test for sprite overflow (more than 64 hardware sprites at once).
    ld a,(NextFreeSprite)
    add a,b
    cp 65
    ret nc

    ; Point DE to SpriteBufferY[NextFreeSprite].
    ld a,(NextFreeSprite)
    ld de,SpriteBufferY
    add a,e
    ld e,a
    ld a,0
    adc a,d
    ld d,a

    ; Apply offsets to MasterY, and write the results to SpriteBufferY.
    ld a,(MasterY)
    ld c,a
    -:
      ld a,(hl)
      add a,c
      cp SPRITE_TERMINATOR
      jp nz,+
        inc a
      +:
      ld (de),a
      inc hl
      inc de
    djnz -

    ; Apply offsets to MasterX, and save results, together with charcodes, in
    ; SpriteBufferXC. HL points to the first x-offset.

    ; Point DE to SpriteBufferY[NextFreeSprite]
    ld a,(NextFreeSprite) ; Double the buffer index because this is a buffer
    add a,a               ; word-sized elements  (x + charcode pairs).
    ld de,SpriteBufferXC
    add a,e               ; Calculate offset using 8-bit math to avoid
    ld e,a                ; disturbing HL and B.
    ld a,0
    adc a,d               ; Apply a possible carry from LSB above.
    ld d,a                ; Now, DE is pointing correctly into SpriteBufferXC.

    ; Apply offsets to MasterX and write results together with charcodes.
    ld a,(MasterX)
    ld c,a
    ld a,(MetaSpriteSize) ; Retrieve size and store it in B for counting.
    ld b,a
    -:
      ; First byte: X.
      ld a,(hl)           ; Get the offset.
      add a,c             ; Apply MasterX to this offset.
      ld (de),a           ; Save it in buffer.
      inc hl              ; Forward both source and destination pointers...
      inc de

      ; Second byte: Character code.
      ld a,(hl)           ; Get the char code.
      ld (de),a           ; Save it directly in buffer.
      inc hl              ; Forward the pointers again...
      inc de
    djnz -

    ld hl,MetaSpriteSize
    ld a,(NextFreeSprite) ; add the size of the processed metasprite
    add a,(hl)
    ld (NextFreeSprite),a
  ret
.ends
