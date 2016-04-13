.include "Base.inc"
.include "Spritelib.inc"
.include "Objectlib.inc"

; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
.ramsection "Main variables" slot 3
  FrameCounter db
  PlayerHandle db
  GargoyleHandle db
  ZombieHandle db
  NextEventIndex db
.ends
; -----------------------------------------------------------------------------
.section "Main" free
; -----------------------------------------------------------------------------
  Palette:
    BackgroundPalette:
      .db $35
      .ds 15 0
    SpritePalette:
      .include "Data/Sprite-Palette.inc"
    SpritePaletteEnd:

  ; Tilebank 2 map:
  .equ PLAYER_1_TILES_START $2000
  .equ ENEMY_1_TILES_START $2400
  .equ ENEMY_2_TILES_START $2600

  ; Make character code indexes for the meta sprite data blocks.
  .equ P1 (PLAYER_1_TILES_START-$2000)/32
  .equ E1 (ENEMY_1_TILES_START-$2000)/32
  .equ E2 (ENEMY_2_TILES_START-$2000)/32

  SwabbyTiles:
    .include "Data/Swabby-Tiles.inc"
  SwabbyTilesEnd:
  GargoyleTiles:
    .include "Data/Gargoyle-Tiles.inc"
  GargoyleTilesEnd:
  ZombieTiles:
    .include "Data/Zombie-Tiles.inc"
  ZombieTilesEnd:

  BatchLoadTable:
    .db (BatchLoadTableEnd-BatchLoadTable-1)/6
    .dw SwabbyTiles PLAYER_1_TILES_START SwabbyTilesEnd-SwabbyTiles
    .dw GargoyleTiles ENEMY_1_TILES_START GargoyleTilesEnd-GargoyleTiles
    .dw ZombieTiles ENEMY_2_TILES_START ZombieTilesEnd-ZombieTiles
  BatchLoadTableEnd:

  .macro Make3x2MetaSprites
    .rept nargs/2
      \2
        .db 6
        .db -4, -4, -4, 4, 4, 4
        .db -8, \1, 0, \1+1, 8, \1+2, -8, \1+3, 0, \1+4, 8, \1+5
        .shift
        .shift
    .endr
  .endm

  .macro Make2x2MetaSprites
    .rept nargs/2
      \2
        .db 4
        .db -4, -4, 4, 4
        .db -4, \1, 4, \1+1, -4, \1+2, 4, \1+3
        .shift
        .shift
    .endr
  .endm

  .macro Make2x3MetaSprites
    .rept nargs/2
      \2
        .db 6
        .db -8, -8, 0, 0, 8, 8
        .db -4, \1, 4, \1+1, -4, \1+2, 4, \1+3, -4, \1+4, 4, \1+5
        .shift
        .shift
    .endr
  .endm

  Make3x2MetaSprites P1, Swabby1:, P1+6, Swabby2:, P1+12, Swabby3:
  Make2x2MetaSprites E1, Gargoyle1:, E1+4, Gargoyle2:
  Make2x3MetaSprites E2, Zombie1:, E2+6, Zombie2:

  SwabbyInitString:
    .db 1                     ; Initial status.
    .db 20 40                 ; Start Y and start X.
    .dw Swabby1               ; MetaSpritePointer.
    .db JOYSTICK_1 1 2        ; Movement type, vertical and horizontal speed.

  GargoyleInitString:
    .db 1
    .db 50 50
    .dw Gargoyle1
    .db JOYSTICK_2 2 2

  ZombieInitString:
    .db 1
    .db 120 120
    .dw Zombie1
    .db SIMPLE_MOVEMENT, 1, 1

  SetupMain:
    ld a,0
    ld b,32
    ld hl,Palette
    call LoadCRam

    ld hl,BatchLoadTable
    call _BatchLoadTiles

    ld a,ENABLE_DISPLAY_ENABLE_FRAME_INTERRUPTS_NORMAL_SPRITES
    ld b,1
    call SetRegister
    ei
    call AwaitFrameInterrupt

    ; Fall through to main loop...?

  Main:
    call AwaitFrameInterrupt
    call LoadSAT

    ld a,(FrameCounter)
    inc a
    ld (FrameCounter),a
    call z,_MakeEvent

    call GetInputPorts

    call ObjectFrame

  jp Main

  _BatchLoadTiles:
    ; Entry: HL = Base address of BatchLoadTable to use
    ld b,(hl)             ; Read number of entries in the BatchLoadTable.
    inc hl
    push hl
    pop ix
    -:
      push bc
        ld l,(ix+0)       ; Get pointer to tile data.
        ld h,(ix+1)
        ld e,(ix+2)       ; Get pointer to destination in vram.
        ld d,(ix+3)
        ld c,(ix+4)       ; Get number of bytes to load.
        ld b,(ix+5)
        call LoadVRam
        ld de,6
        add ix,de         ; Advance the BatchLoadTable index to next entry.
      pop bc
    djnz -                ; Process all elements in BatchLoadTable
  ret

  _MakeEvent:
    ld a,(NextEventIndex)
    add a,a
    ld d,0
    ld e,a
    ld hl,_EventTable
    add hl,de
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    jp (hl)
    ; Refer to event table in the data section above.
    ; .... when the event is over, we jump back to _EndEvents below.
  _EndEvents:
    ld a,(NextEventIndex)
    cp ((_EventTableEnd-_EventTable)/2)-1
    ret z
    inc a
    ld (NextEventIndex),a
  ret

  _EventTable:
    .dw _Event0 _Event1 _Event2
  _EventTableEnd:
    _Event0:
      ld hl,SwabbyInitString
      call CreateObject
      ld (PlayerHandle),a
      jp _EndEvents
    _Event1:
      ld hl,GargoyleInitString
      call CreateObject
      ld (GargoyleHandle),a
      ld hl,ZombieInitString
      call CreateObject
      ld (ZombieHandle),a
      jp _EndEvents
    _Event2:
      nop ; Do nothing... (event handler loops on this last element).
      jp _EndEvents

.ends
