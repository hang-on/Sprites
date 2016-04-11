; *****************************************************************************
;                           OBJECT LIBRARY
; *****************************************************************************
; Note: It builds on top of the Spritelib.

; Definitions for object's movement type.
.equ STATIC     0
.equ JOYSTICK_1 1
.equ JOYSTICK_2 2
.equ VERTICAL   3
.equ HORIZONTAL 4

.ramsection "Objectlib Variables" slot 3
  ObjectAttributeTable dsb 256 ; 16 objects, 16 bytes each. Abbrev. OAT.
    ; 0: Status
    ; 1    : Y
    ; 2    : X
    ; 3-4  : MetaSpritePointer
    ; 5    : Movement
    ; 6    : Vertical speed
    ; 7    : Horizontal speed
    ; 8-15 : Reserved
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
        ;call _LoadElement
        call _LoadElementProto
        or a                ; Clear carry (operation successful).
        ld a,b              ; Return with A holding the index.
        ret
      +:
      ld de,16
      add ix,de
      inc b
      ld a,b
      cp 16
    jp nz,-
    scf                      ; Set carry to indicate error.
  ret

  _LoadElementProto:
    push ix
    pop de
    .rept 15
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
    ld de,16
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
.section "ObjectFrame" free
; -----------------------------------------------------------------------------
  ; Frame-by-frame housekeeping of the objects.
  ObjectFrame:
    ; Start with animation, etc...
    call MoveObjects

    ; Finish by drawing the objects as they look by now.
    call DrawObjects
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
      ld de,16
      add ix,de
    .endr
    call FinalizeMetaSprites
  ret

  _DrawObject:
    ; Entry: IX = Pointer to OAT element.
    ; Uses: AF,B,HL,IX
    ld l,(ix+3)
    ld h,(ix+4)
    ld a,(ix+1)
    ld b,(ix+2)
    call AddMetaSprite
  ret
.ends

; -----------------------------------------------------------------------------
.section "MoveObjects" free
; -----------------------------------------------------------------------------
  ; Process all objects. If an object is active, handle its movement with
  ; settings in Movement, Vertical and Horizontal speed.
  MoveObjects:
    ld hl,ObjectAttributeTable
    .rept 16
      ld a,(hl)
      or a                  ; Zero means object is idle.
      call nz,_MoveObject
      ld de,16
      add hl,de
    .endr
  ret

  _MoveObject:
  ; Entry: HL = Base address of active OAT element.
  ret
.ends
