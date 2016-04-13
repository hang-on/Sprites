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

  SwabbyFlying1:
    .db 6
    .db -4, -4, -4, 4, 4, 4
    .db -8, P1, 0, P1+1, 8, P1+2, -8, P1+3, 0, P1+4, 8, P1+5
  SwabbyFlying2:
    .db 6
    .db -4, -4, -4, 4, 4, 4
    .db -8, P1+6, 0, P1+7, 8, P1+8, -8, P1+9, 0, P1+10, 8, P1+11
  SwabbyFlying3:
    .db 6
    .db -4, -4, -4, 4, 4, 4
    .db -8, P1+12, 0, P1+13, 8, P1+14, -8, P1+15, 0, P1+16, 8, P1+17
  SwabbyInitString:
    .db 1                     ; Initial status.
    .db 20 40                 ; Start Y and start X.
    .dw SwabbyFlying3         ; MetaSpritePointer.
    .db 0 0 0                 ; Movement type, vertical and horizontal speed.

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

  Make2x2MetaSprites E1, GargoyleFlying1:, E1+4, GargoyleFlying2:

  .macro TestMacro
    .rept nargs/2
      \2
        .db \1
      .shift
      .shift
    .endr
  .endm

  TestMacro $ff, MyLabel: $fe, MyLabel2:

  GargoyleMetaSprite:
    .db 4
    .db -4, -4, 4, 4
    .db -4, E1, 4, E1+1, -4, E1+2, 4, E1+3
  GargoyleInitString:
    .db 1
    .db 50 50
    .dw GargoyleFlying1
    .db 0 0 0

  ZombieMetaSprite:
    .db 6
    .db -8, -8, 0, 0, 8, 8
    .db -4, E2, 4, E2+1, -4, E2+2, 4, E2+3, -4, E2+4, 4, E2+5
  ZombieInitString:
    .db 1
    .db 120 120
    .dw ZombieMetaSprite
    .db 0 0 0

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
.ends
