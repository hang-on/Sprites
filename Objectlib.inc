; *****************************************************************************
;                           OBJECT LIBRARY
; *****************************************************************************
; Note: It builds on top of the Spritelib.

; Object Attribute Table definitions.
.equ OAT_ELEMENT_SIZE 64
.equ MAX_OBJECTS 16

.equ OAT_STATUS 0
.equ OAT_Y 1
.equ OAT_X 2
.equ OAT_META_SPRITE_POINTER_LSB 3
.equ OAT_MOVEMENT 5
.equ OAT_VERTICAL_SPEED 6
.equ OAT_HORIZONTAL_SPEED 7
.equ OAT_ANIM_TABLE_BASE_ADDRESS_LSB 8
.equ OAT_ANIM_TABLE_INDEX 10
.equ OAT_ANIM_TIMER 11
.equ OAT_MOVEMENT_PATTERN_POINTER_LSB 12
.equ OAT_PATTERN_TIMER 14
.equ OAT_DELAY_COUNTER 15
.equ OAT_DELAY_VALUE 16
.equ OAT_META_SPRITE_FLAGS 17

; Definitions for object's movement type.
.equ STATIC            0  ; Ignore vertical and horizontal speeds. Stay still.
.equ JOYSTICK_1        1  ; Object is controlled with joystick in port 1.
.equ JOYSTICK_2        2  ; Object is controlled with joystick in port 2.
.equ SIMPLE_MOVEMENT   3  ; Object moves by values in v- and h-speed.
.equ PATTERN           4  ; Object moves according to a pattern.
; NOTE: Be sure to align _MovementTable to these definitions!


.ramsection "Objectlib Variables" slot 3
  ObjectAttributeTable dsb 16*OAT_ELEMENT_SIZE
    ; 0     : Status
    ; 1     : Y
    ; 2     : X
    ; 3-4   : MetaSpritePointer
    ; 5     : Movement
    ; 6     : Vertical speed
    ; 7     : Horizontal speed
    ; 8-9   : AnimTableBaseAddress
    ; 10    : AnimationTableIndex
    ; 11    : AnimationTimer
    ; 12-13 : Movement pattern table pointer.
    ; 14    : Pattern timer.
    ; 15    : DelayCounter - counts from DelayValue to zero.
    ; 16    : DelayValue - how many frames to skip during each movement.
    ; 17    : MetaSpriteFlags - uuuu uulr
    ;           - enable left or right mask (for horizontal clipping).
    ; 18-63 : Unused
.ends

; -----------------------------------------------------------------------------
.section "AnimateObject" free
; -----------------------------------------------------------------------------
  ; Decrement object's animation timer. Load new meta sprite pointer (i.e. a
  ; new frame of animation) if the timer is up.
  ; Entry: IX = Base address of OAT element.
  ; Uses: AF, DE, HL, IX
  AnimateObject:
    ld a,(ix+OAT_ANIM_TIMER)
    or a                  ; Zero: Time to load a new meta sprite pointer.
    call z,_LoadMetaSpriterPointer
    dec (ix+OAT_ANIM_TIMER)
  ret

  _LoadMetaSpriterPointer:
    ; Entry: IX = Pointer to OAT element.
    ld a,(ix+OAT_ANIM_TABLE_INDEX)      ; Get animation table index.
    add a,a
    add a,a           ; Times 4.
    ld d,0
    ld e,a
    ld l,(ix+OAT_ANIM_TABLE_BASE_ADDRESS_LSB)
    ld h,(ix+OAT_ANIM_TABLE_BASE_ADDRESS_LSB+1)
    add hl,de
    ld a,(hl)         ; Read control byte.
    or a
    jp z,+           ; Zero means forward to next frame.
      ld a,-1        ; .. So it wil be incremented to zero below.
      ld (ix+OAT_ANIM_TABLE_INDEX),a
    +:
      inc hl
      ld a,(hl)       ; Read new timer value from table.
      ld (ix+OAT_ANIM_TIMER),a    ; Store it in OAT.
      inc hl
      ld a,(hl)
      ld (ix+OAT_META_SPRITE_POINTER_LSB),a
      inc hl
      ld a,(hl)
      ld (ix+OAT_META_SPRITE_POINTER_LSB+1),a
      inc (ix+OAT_ANIM_TABLE_INDEX)     ; Increment the animation table index.
  ret
.ends

; -----------------------------------------------------------------------------
.section "CreateObject" free
; -----------------------------------------------------------------------------
  ; Find an object with Status = 0 in OAT. Load data into this object from a
  ; string pointed to by HL.
  ; Entry: HL = Pointer to object initialization data.
  ; Exit: A = Table index assigned to this object.
  ; Uses: AF,B,DE,HL,IX
  CreateObject:
    ; Search for the next free object in OAT
    ld ix,ObjectAttributeTable
    ld b,0
    -:
      ld a,(ix+0)
      or a                  ; Zero means object is idle.
      jp nz,+
        call _LoadElement
        or a                ; Clear carry (operation successful).
        ld a,b              ; Return with A holding the index.
        ret
      +:
      ld de,OAT_ELEMENT_SIZE
      add ix,de
      inc b
      ld a,b
      cp MAX_OBJECTS
    jp nz,-
    scf                      ; Set carry to indicate error.
  ret

  _LoadElement:
    push ix
    pop de
    .rept OAT_ELEMENT_SIZE
      ldi
    .endr
  ret
.ends

; -----------------------------------------------------------------------------
.section "DestroyObject" free
; -----------------------------------------------------------------------------
  ; Detroys an object.
  ; Entry: A = Handle of object to destroy.
  ; Uses: AF,B,HL
  DestroyObject:
    ld hl,ObjectAttributeTable
    cp 0
    jp z,+
    ld de,OAT_ELEMENT_SIZE
    ld b,a
    -:
      add hl,de
    djnz -
    +:
    xor a             ; Write zero to signal that this obj. is now 'idle'.
    ld (hl),a
  ret
.ends

; -----------------------------------------------------------------------------
.section "DrawObjects" free
; -----------------------------------------------------------------------------
  ; Utilize the Spritelib for drawing the active objects' metasprites.
  ; Search for the next free object in OAT (from last to first element).
  ; Uses: AF,B,DE,HL,IX
  DrawObjects:
    call BeginMetaSprites
    ld ix,ObjectAttributeTable
    .rept 16
      ld a,(ix+0)
      or a                  ; Zero means object is idle.
      call nz,_DrawObject
      ld de,OAT_ELEMENT_SIZE
      add ix,de
    .endr
  ret

  _DrawObject:
    ; Entry: IX = Pointer to OAT element.
    ; Uses: AF,B,HL,IX
    ld l,(ix+3)
    ld h,(ix+4)
    ld a,(ix+1)
    ld b,(ix+2)
    ld c,(ix+17)          ; The metasprite flags.
    call AddMetaSprite
  ret
.ends


; -----------------------------------------------------------------------------
.section "HandleMask" free
; -----------------------------------------------------------------------------
  ; Check object's mask status, and toggle mask depending on object's
  ; horizontal position and mask status.
  ; Entry: IX = OAT element.
  HandleMasks:
    bit 1,(ix+17)           ; Is left mask on?
    jp nz,_HandleLeftMask
    bit 0,(ix+17)           ; Is right mask on?
    jp nz,_HandleRightMask
    ; else fall through to 'no masks on' below...
    ld a,(ix+2)
    cp 64
    jp nc,+
      set 0,(ix+17)
      ret
    +:
    cp 191
    ret c
    set 1,(ix+17)
  ret

  _HandleLeftMask:
    ld a,(ix+2)
    cp 64
    ret c
    sub 191
    ret nc
    res 1,(ix+17)           ; Player in middle zone, reset left mask.
  ret

  _HandleRightMask:
    ld a,(ix+2)
    cp 191
    ret nc
    sub 64
    ret c
    res 0,(ix+17)
  ret
.ends

; -----------------------------------------------------------------------------
.section "ObjectFrame" free
; -----------------------------------------------------------------------------
  ; Frame-by-frame housekeeping of the objects.
  ; Assume that all registers are used.
  ObjectFrame:
    ld ix,ObjectAttributeTable
    .rept 16                ; Run through all 16 objects.
      ld a,(ix+0)           ; Get the status byte.
      or a                  ; Zero means object is idle.
      jp z,+                ; If idle, then skip forward, else process object.
        call HandleMasks
        call AnimateObject
        call MoveObject

      +:
      ld de,OAT_ELEMENT_SIZE
      add ix,de             ; Point to status byte of next object.
    .endr                   ; Repeat to test and process all objects.

    ; Finish by drawing the objects as they look by now.
    call DrawObjects
  ret
.ends

; -----------------------------------------------------------------------------
.section "MoveObject" free
; -----------------------------------------------------------------------------
  MoveObject:
    ; Entry: IX = Base address of active OAT element.

    ; First handle delay.
    ld a,(ix+16)            ; Get delay value (0 means no delay).
    or a
    jp z,+
      ; Okay, delay is active...
      dec (ix+15)
      ret nz
      ld (ix+15),a          ; A still holds the delay value.
    +:
    ld a,(ix+5)
    add a,a
    ld d,0
    ld e,a
    ld hl,_MovementTable
    add hl,de
    ld a,(hl)
    inc hl
    ld h,(hl)
    ld l,a
    jp (hl)

    _MovementTable:
      .dw _Static, _Joystick1Controlled, _Joystick2Controlled
      .dw _SimpleMovement _PatternMovement

    _Static:
      jp _EndSwitch

    _Joystick1Controlled:
      call IsPlayer1RightPressed
      jp nc,+
        ld a,(ix+7)
        add a,(ix+2)
        ld (ix+2),a
      +:
      call IsPlayer1LeftPressed
      jp nc,+
        ld a,(ix+2)
        sub (ix+7)
        ld (ix+2),a
      +:
      call IsPlayer1UpPressed
      jp nc,+
        ld a,(ix+1)
        sub (ix+6)
        ld (ix+1),a
      +:
      call IsPlayer1DownPressed
      jp nc,+
        ld a,(ix+1)
        add a,(ix+6)
        ld (ix+1),a
      +:
    jp _EndSwitch

    _Joystick2Controlled:
      call IsPlayer2RightPressed
      jp nc,+
        ld a,(ix+7)
        add a,(ix+2)
        ld (ix+2),a
      +:
      call IsPlayer2LeftPressed
      jp nc,+
        ld a,(ix+2)
        sub (ix+7)
        ld (ix+2),a
      +:
      call IsPlayer2UpPressed
      jp nc,+
        ld a,(ix+1)
        sub (ix+6)
        ld (ix+1),a
      +:
      call IsPlayer2DownPressed
      jp nc,+
        ld a,(ix+1)
        add a,(ix+6)
        ld (ix+1),a
      +:
    jp _EndSwitch

    _SimpleMovement:
      ; Simply apply the vertical and horizontal speed to y and x...
      ld a,(ix+1)
      add a,(ix+6)
      ld (ix+1),a
      ld a,(ix+2)
      add a,(ix+7)
      ld (ix+2),a
    jp _EndSwitch

    _PatternMovement:
      ld a,(ix+14)
      or a
      jp nz,+
        debug:
        ld l,(ix+12)
        ld h,(ix+13)
        ld a,(hl)       ; Get vertical speed from pattern table.
        cp $aa          ; If vertical speed is $aa, it means that we should
        call z,_LoadPatternPointer ; load a new value into the pattern pointer.

        ld (ix+6),a     ; Update object's vertical speed.
        inc hl
        ld a,(hl)       ; Get horizontal speed from pattern table.
        ld (ix+7),a     ; Update object's horizontal speed.
        inc hl
        ld a,(hl)       ; Get pattern timer value
        ld (ix+14),a    ; ... and set it.
        inc hl
        ld (ix+12),l    ; Update the pattern table pointer.
        ld (ix+13),h
      +:
      dec (ix+14)       ; Decrement the pattern timer.

      ; Simply apply the vertical and horizontal speed to y and x...
      ld a,(ix+1)
      add a,(ix+6)
      ld (ix+1),a
      ld a,(ix+2)
      add a,(ix+7)
      ld (ix+2),a
    jp _EndSwitch

    _EndSwitch:
  ret

  _LoadPatternPointer:
    ; Read pointer from pattern table (at HL+1 and HL+2), and store it in the
    ; pattern pointer in object's OAT element.
    ; Entry: HL = Pointer to control byte $ff in a pattern table.
    ;        IX = This object on the OAT.
    ; Exit:  A = vertical speed.
    inc hl
    ld a,(hl)
    ld (ix+12),a
    inc hl
    ld a,(hl)
    ld (ix+13),a
    ld l,(ix+12)
    ld h,(ix+13)
    ld a,(hl)       ; Leave with vertical speed in A...
  ret
.ends
