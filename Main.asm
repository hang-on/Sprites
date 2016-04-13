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
  SpritePalette:
    .include "Data/Sprite-Palette.inc"
  SpritePaletteEnd:

  BackgroundPalette:
    .db $35

  SpriteTilesBegin:
    .include "Data/Swabby-Tiles.inc"
    .include "Data/Gargoyle-Tiles.inc"
    .include "Data/Zombie-Tiles.inc"
  SpriteTilesEnd:

  SwabbyMetaSprite:
    .db 6
    .db -4, -4, -4, 4, 4, 4
    .db -8, 0, 0, 1, 8, 2, -8, 3, 0, 4, 8, 5

  GargoyleMetaSprite:
    .db 4
    .db 0 0 8 8
    .db 0 18 8 19 0 20 8 21

  ZombieMetaSprite:
    .db 6
    .db 0 0 8 8 16 16
    .db 0 26 8 27 0 28 8 29 0 30 8 31

  SwabbyInitString:
    .db 1                     ; Initial status.
    .db 20 40                 ; Start Y and start X.
    .dw SwabbyMetaSprite      ; MetaSpritePointer.
    .db 0 0 0                 ; Movement type, vertical and horizontal speed.

  GargoyleInitString:
    .db 1
    .db 50 50
    .dw GargoyleMetaSprite
    .db 0 0 0

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
    ld b,1
    ld hl,BackgroundPalette
    call LoadCRam
    ld a,16
    ld b,SpritePaletteEnd-SpritePalette
    ld hl,SpritePalette
    call LoadCRam

    ld hl,SpriteTilesBegin
    ld de,$2000
    ld bc,SpriteTilesEnd-SpriteTilesBegin
    call LoadVRam

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
