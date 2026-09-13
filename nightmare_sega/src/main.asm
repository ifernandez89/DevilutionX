; ================================================================
; NIGHTMARE: ASHENMOOR — SEGA GENESIS / MEGA DRIVE ARPG DEMO
; 16-Bit Action RPG - Top-Down 3/4 Perspective
; Heroine: The Sorceress (Arcanist)
; 3 Dungeon Levels + The Flayer Boss
; ================================================================

    org     $00000000

; ================================================================
; 1. 68000 VECTOR TABLE ($000000 - $0000FF)
; ================================================================
Vectors:
    dc.l    $00FFE000       ; Initial Stack Pointer (Top of RAM)
    dc.l    EntryPoint      ; Initial Program Counter
    dc.l    DefaultHandler  ; Bus Error
    dc.l    DefaultHandler  ; Address Error
    dc.l    DefaultHandler  ; Illegal Instruction
    dc.l    DefaultHandler  ; Zero Divide
    dc.l    DefaultHandler  ; CHK Instruction
    dc.l    DefaultHandler  ; TRAPV Instruction
    dc.l    DefaultHandler  ; Privilege Violation
    dc.l    DefaultHandler  ; Trace
    dc.l    DefaultHandler  ; Line 1010 Emulator
    dc.l    DefaultHandler  ; Line 1111 Emulator
    dcb.l   12, DefaultHandler ; Reserved by Motorola
    dc.l    DefaultHandler  ; Spurious Interrupt
    dc.l    DefaultHandler  ; Level 1 Interrupt
    dc.l    DefaultHandler  ; Level 2 Interrupt
    dc.l    DefaultHandler  ; Level 3 Interrupt
    dc.l    DefaultHandler  ; Level 4 Interrupt (HBlank)
    dc.l    DefaultHandler  ; Level 5 Interrupt
    dc.l    DefaultHandler  ; Level 6 Interrupt (VBlank)
    dc.l    DefaultHandler  ; Level 7 Interrupt
    dcb.l   16, DefaultHandler ; TRAP #0 - #15
    dcb.l   16, DefaultHandler ; Reserved by Motorola

; ================================================================
; 2. STANDARD SEGA GENESIS ROM HEADER ($000100 - $0001FF)
; ================================================================
Header:
    dc.b    "SEGA MEGA DRIVE "              ; Hardware System ID (16B)
    dc.b    "(C)NIGHTMARE2026"              ; Copyright / Release Date (16B)
    dc.b    "NIGHTMARE: ASHENMOOR SORCERESS ARPG DEMO        " ; Domestic Title (48B)
    dc.b    "NIGHTMARE: ASHENMOOR SORCERESS ARPG DEMO        " ; Overseas Title (48B)
    dc.b    "GM NIGHT001-00"                ; Serial Number (14B)
    dc.w    $0000                           ; Checksum (Patched by build script)
    dc.b    "J6              "              ; Controller Support (16B)
    dc.l    $00000000                       ; ROM Start Address
    dc.l    $0007FFFF                       ; ROM End Address (512 KB)
    dc.l    $00FF0000                       ; RAM Start Address
    dc.l    $00FFFFFF                       ; RAM End Address
    dc.b    "RA", $F8, $20                  ; Backup SRAM Support
    dc.l    $00200000                       ; SRAM Start Address
    dc.l    $002001FF                       ; SRAM End Address
    dcb.b   12, " "                         ; Modem Support
    dcb.b   40, " "                         ; Developer Notes
    dc.b    "JUE             "              ; Country / Region Support (16B)

; ================================================================
; 3. RAM VARIABLES DEFINITIONS ($00FF0000+)
; ================================================================
RAM_Base            equ $00FF0000

Pad1_State          equ RAM_Base+$02        ; Word: D-Pad and Buttons held
Pad1_Press          equ RAM_Base+$04        ; Word: Newly pressed buttons
Frame_Counter       equ RAM_Base+$06        ; Word: Global ticks
Current_Level       equ RAM_Base+$08        ; Word: 0=Level 1, 1=Level 2, 2=Level 3
Game_State          equ RAM_Base+$0A        ; Word: 0=Play, 1=NextLevel, 2=Victory, 3=GameOver
Game_Timer          equ RAM_Base+$0C        ; Word: Timer for state transitions
RNG_Seed            equ RAM_Base+$0E        ; Word: PRNG state
Audio_Timer         equ RAM_Base+$10        ; Word: Timer to auto-silence PSG

; Player Data
Player_X            equ RAM_Base+$20        ; Word (pixel X: 0-319)
Player_Y            equ RAM_Base+$22        ; Word (pixel Y: 0-223)
Player_Facing       equ RAM_Base+$24        ; Word: 0=Down, 1=Up, 2=Left, 3=Right
Player_AnimFrame    equ RAM_Base+$26        ; Word: 0=Idle, 1=Walk, 2=Cast
Player_AnimTimer    equ RAM_Base+$28        ; Word
Player_HP           equ RAM_Base+$2A        ; Word (0-100)
Player_MaxHP        equ RAM_Base+$2C        ; Word (100)
Player_MP           equ RAM_Base+$2E        ; Word (0-100)
Player_MaxMP        equ RAM_Base+$30        ; Word (100)
Player_Potions      equ RAM_Base+$32        ; Word (Count)
Player_CastCooldown equ RAM_Base+$34        ; Word
Player_HitInvuln    equ RAM_Base+$36        ; Word (invulnerability flash)
Stairs_X            equ RAM_Base+$38        ; Word
Stairs_Y            equ RAM_Base+$3A        ; Word
Monster_Count       equ RAM_Base+$3C        ; Word (Active monsters)

; Projectiles (Up to 4 active) - 16 bytes each
Proj_Base           equ RAM_Base+$40
; Struct: Active(w), X(w), Y(w), VX(w), VY(w), Life(w), Type(w), Pad(w) = 16B
MAX_PROJ            equ 4

; Monsters (Up to 8 active) - 24 bytes each
Mon_Base            equ RAM_Base+$90
; Struct: Active(w), Type(w), X(w), Y(w), HP(w), MaxHP(w), AnimTimer(w), HitTimer(w), AttackCooldown(w), Facing(w), Speed(w), Pad(w) = 24B
MAX_MON             equ 8

; Hardware Sprite Buffer (SAT in RAM) - 80 sprites x 8 bytes = 640 bytes
SAT_Buffer          equ RAM_Base+$1000
Active_Sprites_Count equ RAM_Base+$1300

; ================================================================
; 4. HARDWARE PORTS & CONSTANTS
; ================================================================
VDP_DATA            equ $00C00000
VDP_CTRL            equ $00C00004
PSG_PORT            equ $00C00011
TMSS_REG            equ $00A14000
PORT_A_DATA         equ $00A10003
PORT_A_CTRL         equ $00A10009
Z80_BUSREQ          equ $00A11100
Z80_RESET           equ $00A11200

; Controller 1 Bitmasks
BTN_UP              equ $0001
BTN_DOWN            equ $0002
BTN_LEFT            equ $0004
BTN_RIGHT           equ $0008
BTN_B               equ $0010
BTN_C               equ $0020
BTN_A               equ $0040
BTN_START           equ $0080

; ================================================================
; 5. ENTRY POINT & HARDWARE INITIALIZATION
; ================================================================
EntryPoint:
    ; Mask interrupts (We use deterministic VDP status polling)
    move.w  #$2700, sr

    ; 1. TMSS Unlock for Model 2 / Model 3
    move.b  $00A10001, d0       ; Read Hardware Version
    andi.b  #$0F, d0
    beq     .skip_tmss
    move.l  #'SEGA', TMSS_REG   ; Write 'SEGA'
.skip_tmss:

    ; 2. Initialize Z80 Bus (Halt Z80 so it doesn't access bus)
    move.w  #$0100, Z80_BUSREQ
    move.w  #$0100, Z80_RESET
.wait_z80:
    btst    #0, Z80_BUSREQ
    bne     .wait_z80

    ; 3. Clear Work RAM ($00FF0000 - $00FFFFFF)
    lea     RAM_Base, a0
    move.w  #$3FFF, d0          ; 16,384 longwords = 64 KB
    moveq   #0, d1
.clear_ram:
    move.l  d1, (a0)+
    dbf     d0, .clear_ram

    ; 4. Initialize Controller Port 1
    move.b  #$40, PORT_A_CTRL   ; Set TH pin as output

    ; 5. Initialize VDP Registers
    lea     VDP_Init_Data, a0
    move.w  #VDP_Init_Count-1, d0
    move.l  #VDP_CTRL, a1
.init_vdp:
    move.w  (a0)+, (a1)
    dbf     d0, .init_vdp

    ; 6. Clear VRAM (64 KB)
    move.l  #$40000000, (a1)    ; VRAM write at $0000
    move.w  #$3FFF, d0
    moveq   #0, d1
    move.l  #VDP_DATA, a0
.clear_vram:
    move.l  d1, (a0)
    dbf     d0, .clear_vram

    ; 7. Clear CRAM (Palettes)
    move.l  #$C0000000, (a1)    ; CRAM write at $0000
    move.w  #31, d0
.clear_cram:
    move.l  #0, (a0)
    dbf     d0, .clear_cram

    ; 8. Load Palettes from ROM into CRAM
    move.l  #$C0000000, (a1)
    lea     Palettes_Data, a2
    move.w  #(Palettes_Data_End-Palettes_Data)/2-1, d0
.load_palettes:
    move.w  (a2)+, (a0)
    dbf     d0, .load_palettes

    ; 9. Load Tile Patterns into VRAM ($0000 - $1FFF)
    move.l  #$40000000, (a1)
    lea     DungeonTiles, a2
    move.w  #(FxHudTiles_End-DungeonTiles)/4-1, d0
.load_tiles:
    move.l  (a2)+, (a0)
    dbf     d0, .load_tiles

    ; 10. Silence all PSG Channels immediately!
    move.b  #$9F, PSG_PORT      ; Ch 0 Silence
    move.b  #$BF, PSG_PORT      ; Ch 1 Silence
    move.b  #$DF, PSG_PORT      ; Ch 2 Silence
    move.b  #$FF, PSG_PORT      ; Noise Silence
    clr.w   Audio_Timer

    ; 11. Initialize Game Variables
    clr.w   Current_Level
    clr.w   Game_State
    move.w  #100, Player_HP
    move.w  #100, Player_MaxHP
    move.w  #100, Player_MP
    move.w  #100, Player_MaxMP
    move.w  #3, Player_Potions
    move.w  #$5A73, RNG_Seed

    bsr     Load_Current_Dungeon

    ; Enable VDP Display
    move.w  #$8174, VDP_CTRL    ; Reg 1: Display ON, DMA ON, Mode 5, 224 lines

; ================================================================
; 6. MAIN GAME LOOP (Synchronized to VBlank via VDP status)
; ================================================================
MainLoop:
    ; 1. Wait for VBlank to start
    bsr     Wait_VBlank

    ; 2. Transfer Sprite Table (SAT) to VRAM $F000 during VBlank
    ; VDP Command for address $F000 is $70000003
    move.l  #$70000003, VDP_CTRL
    lea     SAT_Buffer, a0
    move.w  #159, d0            ; 160 longwords = 640 bytes (80 hardware sprites)
    move.l  #VDP_DATA, a1
.copy_sat:
    move.l  (a0)+, (a1)
    dbf     d0, .copy_sat

    ; 3. Audio Timer & Silence Check
    bsr     Update_Audio

    ; 4. Advance Global Frame Counter & PRNG
    addq.w  #1, Frame_Counter
    move.w  RNG_Seed, d0
    rol.w   #7, d0
    add.w   #$41C7, d0
    move.w  d0, RNG_Seed

    ; 5. Read Controller Input
    bsr     Read_Controller

    ; 6. Process State
    move.w  Game_State, d0
    beq     .state_play
    cmpi.w  #1, d0
    beq     .state_next_level
    cmpi.w  #2, d0
    beq     .state_victory
    bra     .state_game_over

.state_play:
    bsr     Update_Player
    bsr     Update_Projectiles
    bsr     Update_Monsters
    bsr     Build_Sprite_Table
    bra     MainLoop

.state_next_level:
    addq.w  #1, Game_Timer
    cmpi.w  #50, Game_Timer
    blt     MainLoop
    clr.w   Game_Timer
    addq.w  #1, Current_Level
    cmpi.w  #3, Current_Level
    bge     .trigger_victory
    bsr     Load_Current_Dungeon
    clr.w   Game_State
    bra     MainLoop
.trigger_victory:
    move.w  #2, Game_State
    bra     MainLoop

.state_victory:
    ; Press Start to restart
    move.w  Pad1_Press, d0
    andi.w  #BTN_START, d0
    beq     MainLoop
    clr.w   Current_Level
    clr.w   Game_State
    move.w  #100, Player_HP
    move.w  #100, Player_MP
    move.w  #3, Player_Potions
    bsr     Load_Current_Dungeon
    bra     MainLoop

.state_game_over:
    move.w  Pad1_Press, d0
    andi.w  #BTN_START, d0
    beq     MainLoop
    move.w  #100, Player_HP
    move.w  #100, Player_MP
    move.w  #2, Player_Potions
    clr.w   Game_State
    bsr     Load_Current_Dungeon
    bra     MainLoop

; ================================================================
; 7. VBLANK WAIT ROUTINE (Deterministic Hardware Status Polling)
; ================================================================
Wait_VBlank:
    move.l  #VDP_CTRL, a6
.wait_off:
    move.w  (a6), d0
    btst    #3, d0              ; Bit 3 = 1 during VBlank
    bne     .wait_off           ; Wait until previous VBlank ends
.wait_on:
    move.w  (a6), d0
    btst    #3, d0
    beq     .wait_on            ; Wait until new VBlank starts
    rts

; ================================================================
; 8. AUDIO UPDATE (Auto-Silence PSG after brief duration)
; ================================================================
Update_Audio:
    tst.w   Audio_Timer
    beq     .audio_ok
    subq.w  #1, Audio_Timer
    bne     .audio_ok
    ; Silence all channels
    move.b  #$9F, PSG_PORT
    move.b  #$BF, PSG_PORT
    move.b  #$DF, PSG_PORT
    move.b  #$FF, PSG_PORT
.audio_ok:
    rts

; ================================================================
; 9. CONTROLLER READING ROUTINE
; ================================================================
Read_Controller:
    ; Step 1: TH = 1 -> Read C, B, Right, Left, Down, Up
    move.b  #$40, PORT_A_DATA
    nop
    nop
    move.b  PORT_A_DATA, d0

    ; Step 2: TH = 0 -> Read Start, A
    move.b  #$00, PORT_A_DATA
    nop
    nop
    move.b  PORT_A_DATA, d1

    ; Buttons are active LOW. Invert so 1 = pressed.
    not.b   d0
    not.b   d1

    andi.w  #$3F, d0            ; [0 0 C B R L D U]
    andi.w  #$30, d1            ; [0 0 Start A 0 0 0 0]
    lsl.w   #2, d1              ; shift Start/A to bits 7 and 6
    or.w    d1, d0              ; d0 = [Start A C B R L D U]

    ; Calculate newly pressed buttons
    move.w  Pad1_State, d1
    move.w  d0, Pad1_State
    eor.w   d1, d0
    and.w   Pad1_State, d0
    move.w  d0, Pad1_Press
    rts

; ================================================================
; 10. PLAYER UPDATE (The Sorceress)
; ================================================================
Update_Player:
    ; 1. Mana passive regeneration
    move.w  Frame_Counter, d0
    andi.w  #$0F, d0
    bne     .no_mana_regen
    move.w  Player_MP, d0
    cmp.w   Player_MaxMP, d0
    bge     .no_mana_regen
    addq.w  #1, Player_MP
.no_mana_regen:

    ; 2. Decrement timers
    tst.w   Player_CastCooldown
    beq     .no_cd
    subq.w  #1, Player_CastCooldown
.no_cd:
    tst.w   Player_HitInvuln
    beq     .no_invuln
    subq.w  #1, Player_HitInvuln
.no_invuln:

    ; 3. Check Healing Potion (Button A)
    move.w  Pad1_Press, d0
    andi.w  #BTN_A, d0
    beq     .no_potion
    tst.w   Player_Potions
    beq     .no_potion
    subq.w  #1, Player_Potions
    add.w   #45, Player_HP
    move.w  Player_MaxHP, d1
    cmp.w   Player_HP, d1
    bge     .pot_cap
    move.w  d1, Player_HP
.pot_cap:
    bsr     Play_Potion_SFX
.no_potion:

    ; 4. Check Spell Cast (Button B: Bolt, Button C: Nova)
    move.w  Pad1_Press, d0
    andi.w  #BTN_B, d0
    beq     .check_nova

    ; Cast Bolt (Cost: 4 MP)
    tst.w   Player_CastCooldown
    bne     .check_nova
    cmpi.w  #4, Player_MP
    blt     .check_nova
    subq.w  #4, Player_MP
    move.w  #14, Player_CastCooldown
    move.w  #2, Player_AnimFrame
    move.w  #10, Player_AnimTimer
    bsr     Spawn_Player_Bolt
    bsr     Play_Cast_SFX
    bra     .movement

.check_nova:
    move.w  Pad1_Press, d0
    andi.w  #BTN_C, d0
    beq     .movement

    ; Cast Nova (Cost: 18 MP)
    tst.w   Player_CastCooldown
    bne     .movement
    cmpi.w  #18, Player_MP
    blt     .movement
    sub.w   #18, Player_MP
    move.w  #20, Player_CastCooldown
    move.w  #2, Player_AnimFrame
    move.w  #15, Player_AnimTimer
    bsr     Cast_Arcane_Nova
    bsr     Play_Cast_SFX

.movement:
    tst.w   Player_AnimTimer
    beq     .eval_move
    subq.w  #1, Player_AnimTimer
    bne     .eval_move
    clr.w   Player_AnimFrame

.eval_move:
    move.w  Pad1_State, d0
    move.w  Player_X, d1
    move.w  Player_Y, d2
    moveq   #0, d3

    ; Up
    btst    #0, d0
    beq     .not_up
    subq.w  #2, d2
    move.w  #1, Player_Facing
    moveq   #1, d3
.not_up:
    ; Down
    btst    #1, d0
    beq     .not_down
    addq.w  #2, d2
    clr.w   Player_Facing
    moveq   #1, d3
.not_down:
    ; Left
    btst    #2, d0
    beq     .not_left
    subq.w  #2, d1
    move.w  #2, Player_Facing
    moveq   #1, d3
.not_left:
    ; Right
    btst    #3, d0
    beq     .not_right
    addq.w  #2, d1
    move.w  #3, Player_Facing
    moveq   #1, d3
.not_right:

    ; Animation toggle
    tst.w   d3
    beq     .standing
    cmpi.w  #2, Player_AnimFrame
    beq     .apply_move
    move.w  Frame_Counter, d4
    lsr.w   #3, d4
    andi.w  #1, d4
    move.w  d4, Player_AnimFrame
    bra     .apply_move
.standing:
    cmpi.w  #2, Player_AnimFrame
    beq     .apply_move
    clr.w   Player_AnimFrame

.apply_move:
    ; Clamp to room walls
    cmpi.w  #24, d1
    bge     .x_min_ok
    move.w  #24, d1
.x_min_ok:
    cmpi.w  #288, d1
    ble     .x_max_ok
    move.w  #288, d1
.x_max_ok:
    cmpi.w  #26, d2
    bge     .y_min_ok
    move.w  #26, d2
.y_min_ok:
    cmpi.w  #186, d2
    ble     .y_max_ok
    move.w  #186, d2
.y_max_ok:

    move.w  d1, Player_X
    move.w  d2, Player_Y

    ; Check Stairs Down
    move.w  Stairs_X, d4
    tst.w   d4
    beq     .no_stairs
    sub.w   d1, d4
    bpl     .abs_dx
    neg.w   d4
.abs_dx:
    cmpi.w  #14, d4
    bgt     .no_stairs

    move.w  Stairs_Y, d5
    sub.w   d2, d5
    bpl     .abs_dy
    neg.w   d5
.abs_dy:
    cmpi.w  #14, d5
    bgt     .no_stairs

    ; On Stairs! Next level
    move.w  #1, Game_State
    clr.w   Game_Timer
    bsr     Play_LevelClear_SFX
.no_stairs:
    rts

; ================================================================
; 11. PROJECTILES SYSTEM
; ================================================================
Spawn_Player_Bolt:
    lea     Proj_Base, a0
    moveq   #MAX_PROJ-1, d0
.find_slot:
    tst.w   (a0)
    beq     .found_slot
    lea     16(a0), a0
    dbf     d0, .find_slot
    rts

.found_slot:
    move.w  #1, (a0)
    move.w  Player_X, d1
    move.w  Player_Y, d2
    addq.w  #4, d1
    addq.w  #4, d2
    move.w  d1, 2(a0)
    move.w  d2, 4(a0)
    move.w  #45, 10(a0)
    move.w  #15, 12(a0)

    clr.w   6(a0)
    clr.w   8(a0)
    move.w  Player_Facing, d3
    beq     .facing_down
    cmpi.w  #1, d3
    beq     .facing_up
    cmpi.w  #2, d3
    beq     .facing_left

    ; Right
    move.w  #4, 6(a0)
    rts
.facing_down:
    move.w  #4, 8(a0)
    rts
.facing_up:
    move.w  #-4, 8(a0)
    rts
.facing_left:
    move.w  #-4, 6(a0)
    rts

Cast_Arcane_Nova:
    lea     Mon_Base, a0
    moveq   #MAX_MON-1, d0
.nova_loop:
    tst.w   (a0)
    beq     .next_mon
    move.w  4(a0), d1
    sub.w   Player_X, d1
    bpl     .abs_nx
    neg.w   d1
.abs_nx:
    cmpi.w  #44, d1
    bgt     .next_mon

    move.w  6(a0), d2
    sub.w   Player_Y, d2
    bpl     .abs_ny
    neg.w   d2
.abs_ny:
    cmpi.w  #44, d2
    bgt     .next_mon

    sub.w   #30, 8(a0)
    move.w  #8, 14(a0)
    bsr     Play_Hit_SFX
    tst.w   8(a0)
    bgt     .next_mon
    clr.w   (a0)
    bsr     Play_Kill_SFX

.next_mon:
    lea     24(a0), a0
    dbf     d0, .nova_loop
    rts

Update_Projectiles:
    lea     Proj_Base, a0
    moveq   #MAX_PROJ-1, d0
.proj_loop:
    tst.w   (a0)
    beq     .next_proj

    subq.w  #1, 10(a0)
    ble     .kill_proj

    move.w  2(a0), d1
    add.w   6(a0), d1
    move.w  4(a0), d2
    add.w   8(a0), d2

    cmpi.w  #16, d1
    ble     .kill_proj
    cmpi.w  #304, d1
    bge     .kill_proj
    cmpi.w  #16, d2
    ble     .kill_proj
    cmpi.w  #208, d2
    bge     .kill_proj

    move.w  d1, 2(a0)
    move.w  d2, 4(a0)

    ; Collision against monsters
    lea     Mon_Base, a1
    moveq   #MAX_MON-1, d3
.mon_hit_check:
    tst.w   (a1)
    beq     .next_mon_hit

    move.w  4(a1), d4
    sub.w   d1, d4
    bpl     .p_abs_mx
    neg.w   d4
.p_abs_mx:
    cmpi.w  #14, d4
    bgt     .next_mon_hit

    move.w  6(a1), d5
    sub.w   d2, d5
    bpl     .p_abs_my
    neg.w   d5
.p_abs_my:
    cmpi.w  #16, d5
    bgt     .next_mon_hit

    ; Hit!
    move.w  12(a0), d6
    sub.w   d6, 8(a1)
    move.w  #8, 14(a1)
    bsr     Play_Hit_SFX

    tst.w   8(a1)
    bgt     .kill_proj
    clr.w   (a1)
    bsr     Play_Kill_SFX

    cmpi.w  #2, 2(a1)           ; Was it Boss?
    bne     .check_potion_drop
    move.w  #2, Game_State      ; Victory!
    bra     .kill_proj

.check_potion_drop:
    move.w  RNG_Seed, d7
    andi.w  #$03, d7
    bne     .kill_proj
    addq.w  #1, Player_Potions

.kill_proj:
    clr.w   (a0)
    bra     .next_proj

.next_mon_hit:
    lea     24(a1), a1
    dbf     d3, .mon_hit_check

.next_proj:
    lea     16(a0), a0
    dbf     d0, .proj_loop
    rts

; ================================================================
; 12. MONSTERS UPDATE
; ================================================================
Update_Monsters:
    lea     Mon_Base, a0
    moveq   #MAX_MON-1, d0
.mon_ai_loop:
    tst.w   (a0)
    beq     .next_ai_mon

    tst.w   14(a0)
    beq     .no_hit_timer
    subq.w  #1, 14(a0)
.no_hit_timer:

    tst.w   16(a0)
    beq     .no_atk_cd
    subq.w  #1, 16(a0)
.no_atk_cd:

    move.w  2(a0), d7
    move.w  4(a0), d1
    move.w  6(a0), d2
    move.w  Player_X, d5
    sub.w   d1, d5              ; DX
    move.w  Player_Y, d6
    sub.w   d2, d6              ; DY

    moveq   #1, d3
    cmpi.w  #1, d7              ; Ghoul
    beq     .fast_speed
    cmpi.w  #2, d7              ; Boss
    bne     .apply_ai
.fast_speed:
    moveq   #1, d3
    bra     .chase_x

.apply_ai:
    move.w  Frame_Counter, d4
    btst    #0, d4
    bne     .skip_step

.chase_x:
    cmpi.w  #-2, d5
    bge     .try_right
    sub.w   d3, d1
    move.w  #2, 18(a0)
    bra     .chase_y
.try_right:
    cmpi.w  #2, d5
    ble     .chase_y
    add.w   d3, d1
    move.w  #3, 18(a0)

.chase_y:
    cmpi.w  #-2, d6
    bge     .try_down
    sub.w   d3, d2
    move.w  #1, 18(a0)
    bra     .finish_move
.try_down:
    cmpi.w  #2, d6
    ble     .finish_move
    add.w   d3, d2
    clr.w   18(a0)

.finish_move:
    move.w  d1, 4(a0)
    move.w  d2, 6(a0)

.skip_step:
    ; Attack range check
    move.w  d5, d1
    bpl     .abs_mdx
    neg.w   d1
.abs_mdx:
    cmpi.w  #14, d1
    bgt     .next_ai_mon

    move.w  d6, d2
    bpl     .abs_mdy
    neg.w   d2
.abs_mdy:
    cmpi.w  #14, d2
    bgt     .next_ai_mon

    tst.w   16(a0)
    bne     .next_ai_mon
    tst.w   Player_HitInvuln
    bne     .next_ai_mon

    move.w  #35, 16(a0)
    move.w  #30, Player_HitInvuln

    move.w  #8, d1
    cmpi.w  #1, d7
    bne     .not_ghoul
    move.w  #12, d1
.not_ghoul:
    cmpi.w  #2, d7
    bne     .apply_player_dmg
    move.w  #22, d1

.apply_player_dmg:
    sub.w   d1, Player_HP
    bsr     Play_PlayerHurt_SFX
    tst.w   Player_HP
    bgt     .next_ai_mon
    clr.w   Player_HP
    move.w  #3, Game_State

.next_ai_mon:
    lea     24(a0), a0
    dbf     d0, .mon_ai_loop
    rts

; ================================================================
; 13. HARDWARE SPRITE TABLE BUILDER
; ================================================================
Build_Sprite_Table:
    lea     SAT_Buffer, a0
    moveq   #0, d7              ; Sprite counter

    ; -------------------------------------------------------------
    ; Sprite: Player (Sorceress) - 16x24 (2 tiles wide x 3 tiles high = Size $06)
    ; -------------------------------------------------------------
    move.w  Player_HitInvuln, d0
    andi.w  #4, d0
    bne     .skip_player

    move.w  #47, d1             ; Base: Front Idle
    move.w  Player_Facing, d2
    cmpi.w  #1, d2
    bne     .check_side
    move.w  #59, d1             ; Back Walk
    bra     .got_sorc_tile

.check_side:
    cmpi.w  #2, d2
    beq     .sorc_side
    cmpi.w  #3, d2
    bne     .check_anim
.sorc_side:
    move.w  #65, d1             ; Side Walk
    bra     .got_sorc_tile

.check_anim:
    move.w  Player_AnimFrame, d3
    cmpi.w  #2, d3
    bne     .check_walk
    move.w  #71, d1             ; Cast
    bra     .got_sorc_tile
.check_walk:
    cmpi.w  #1, d3
    bne     .got_sorc_tile
    move.w  #53, d1             ; Walk

.got_sorc_tile:
    ; Y (+128)
    move.w  Player_Y, d0
    add.w   #128, d0
    move.w  d0, (a0)+

    ; Word 1: Size $06 (2x3 tiles) in high byte, Link (d7+1) in low byte
    move.w  d7, d0
    addq.w  #1, d0
    andi.w  #$007F, d0
    ori.w   #$0600, d0          ; Size 2x3 = $06
    move.w  d0, (a0)+

    ; Word 2: Priority ($8000) + Pal 1 ($2000) = $A000
    move.w  #$A000, d0
    cmpi.w  #2, Player_Facing   ; Left HFlip
    bne     .no_hflip
    ori.w   #$0800, d0
.no_hflip:
    or.w    d1, d0
    move.w  d0, (a0)+

    ; X (+128)
    move.w  Player_X, d0
    add.w   #128, d0
    move.w  d0, (a0)+
    addq.w  #1, d7

.skip_player:

    ; -------------------------------------------------------------
    ; Sprites: Active Monsters
    ; -------------------------------------------------------------
    lea     Mon_Base, a1
    moveq   #MAX_MON-1, d6
.mon_sprite_loop:
    tst.w   (a1)
    beq     .next_mon_spr

    move.w  2(a1), d1           ; Type: 0=Skeleton, 1=Ghoul, 2=Boss
    cmpi.w  #2, d1
    beq     .boss_sprite

    ; Y (+128)
    move.w  6(a1), d0
    add.w   #128, d0
    move.w  d0, (a0)+

    move.w  d7, d0
    addq.w  #1, d0
    andi.w  #$007F, d0

    cmpi.w  #1, d1              ; Ghoul?
    beq     .ghoul_spr_config

    ; Skeleton: Size 2x3 ($0600), Pal 2 ($4000)
    ori.w   #$0600, d0
    move.w  d0, (a0)+
    move.w  #$C000, d0          ; Pri + Pal 2
    move.w  #77, d2
    move.w  d0, (a0)+
    bra     .mon_x_pos

.ghoul_spr_config:
    ; Ghoul: Size 2x2 ($0500), Pal 2 ($4000)
    ori.w   #$0500, d0
    move.w  d0, (a0)+
    move.w  #$C000, d0
    ori.w   #89, d0
    move.w  d0, (a0)+

.mon_x_pos:
    move.w  4(a1), d0
    add.w   #128, d0
    move.w  d0, (a0)+
    addq.w  #1, d7
    bra     .next_mon_spr

.boss_sprite:
    ; The Flayer Boss: Size 4x4 ($0F00), Pal 3 ($6000)
    move.w  6(a1), d0
    add.w   #128, d0
    move.w  d0, (a0)+

    move.w  d7, d0
    addq.w  #1, d0
    andi.w  #$007F, d0
    ori.w   #$0F00, d0          ; Size 4x4
    move.w  d0, (a0)+

    move.w  #$E000, d0          ; Pri + Pal 3
    ori.w   #93, d0             ; Tile 93
    move.w  d0, (a0)+

    move.w  4(a1), d0
    add.w   #128, d0
    move.w  d0, (a0)+
    addq.w  #1, d7

.next_mon_spr:
    lea     24(a1), a1
    dbf     d6, .mon_sprite_loop

    ; -------------------------------------------------------------
    ; Sprites: Projectiles (Size 1x1 = $0000)
    ; -------------------------------------------------------------
    lea     Proj_Base, a1
    moveq   #MAX_PROJ-1, d6
.proj_spr_loop:
    tst.w   (a1)
    beq     .next_proj_spr

    move.w  4(a1), d0
    add.w   #128, d0
    move.w  d0, (a0)+

    move.w  d7, d0
    addq.w  #1, d0
    andi.w  #$007F, d0
    ori.w   #$0000, d0          ; Size 1x1
    move.w  d0, (a0)+

    move.w  #$E000, d0          ; Pri + Pal 3
    ori.w   #109, d0            ; Tile 109 = Arcane Bolt
    move.w  d0, (a0)+

    move.w  2(a1), d0
    add.w   #128, d0
    move.w  d0, (a0)+
    addq.w  #1, d7

.next_proj_spr:
    lea     16(a1), a1
    dbf     d6, .proj_spr_loop

    ; -------------------------------------------------------------
    ; Sprites: HUD Life Orb (Left) & Mana Orb (Right)
    ; -------------------------------------------------------------
    ; Life Orb: Y=200, X=8, Size 2x2 ($0500), Tile 112, Pal 3
    move.w  #200+128, (a0)+
    move.w  d7, d0
    addq.w  #1, d0
    andi.w  #$007F, d0
    ori.w   #$0500, d0
    move.w  d0, (a0)+
    move.w  #$E070, (a0)+       ; Pri + Pal 3 + Tile 112
    move.w  #8+128, (a0)+
    addq.w  #1, d7

    ; Mana Orb: Y=200, X=296, Size 2x2 ($0500), Tile 116, Pal 3
    move.w  #200+128, (a0)+
    move.w  #$0500, (a0)+       ; Size 2x2, Link = 0 (LAST SPRITE!)
    move.w  #$E074, (a0)+       ; Pri + Pal 3 + Tile 116
    move.w  #296+128, (a0)+
    addq.w  #1, d7

    move.w  d7, Active_Sprites_Count
    rts

; ================================================================
; 14. DUNGEON MAP LOADING ROUTINE
; ================================================================
Load_Current_Dungeon:
    ; 1. Clear SAT Buffer
    lea     SAT_Buffer, a0
    move.w  #159, d0
.clr_sat:
    clr.l   (a0)+
    dbf     d0, .clr_sat

    ; 2. Clear Monsters & Projectiles
    lea     Mon_Base, a0
    move.w  #MAX_MON*6-1, d0
.clr_mon:
    clr.l   (a0)+
    dbf     d0, .clr_mon

    lea     Proj_Base, a0
    move.w  #MAX_PROJ*4-1, d0
.clr_proj:
    clr.l   (a0)+
    dbf     d0, .clr_proj

    ; 3. Fetch Level Pointers
    move.w  Current_Level, d0
    lsl.w   #3, d0
    lea     Map_Pointers, a1
    move.l  0(a1, d0.w), a2     ; Tile Data Ptr
    move.l  4(a1, d0.w), a3     ; Spawn Data Ptr

    ; 4. Write Tiles to VRAM Plane A ($C000)
    move.l  #VDP_CTRL, a4
    move.l  #VDP_DATA, a5

    moveq   #28-1, d1           ; 28 rows
    move.l  #$40000003, d2      ; Plane A base at $C000

.row_loop:
    move.l  d2, (a4)
    moveq   #40-1, d3           ; 40 columns
.col_loop:
    move.w  (a2)+, (a5)
    dbf     d3, .col_loop

    add.l   #$00800000, d2      ; Next plane row in 64-wide plane
    dbf     d1, .row_loop

    ; 5. Set Spawns
    move.w  (a3)+, Player_X
    move.w  (a3)+, Player_Y
    move.w  (a3)+, Stairs_X
    move.w  (a3)+, Stairs_Y
    move.w  (a3)+, d4
    move.w  d4, Monster_Count
    beq     .no_mon_spawns

    lea     Mon_Base, a0
    subq.w  #1, d4
.spawn_mon_loop:
    move.w  #1, (a0)
    move.w  4(a3), 2(a0)        ; Type
    move.w  0(a3), 4(a0)        ; X
    move.w  2(a3), 6(a0)        ; Y
    addq.l  #6, a3

    move.w  2(a0), d5
    cmpi.w  #2, d5              ; Boss?
    beq     .set_boss_hp
    cmpi.w  #1, d5              ; Ghoul?
    beq     .set_ghoul_hp
    move.w  #25, 8(a0)
    move.w  #25, 10(a0)
    bra     .mon_spawn_next
.set_ghoul_hp:
    move.w  #35, 8(a0)
    move.w  #35, 10(a0)
    bra     .mon_spawn_next
.set_boss_hp:
    move.w  #180, 8(a0)
    move.w  #180, 10(a0)

.mon_spawn_next:
    lea     24(a0), a0
    dbf     d4, .spawn_mon_loop

.no_mon_spawns:
    rts

; ================================================================
; 15. SOUND FX ROUTINES (Clean, Punchy, with Auto-Decay)
; ================================================================
Play_Cast_SFX:
    ; Tone sweep on Channel 0
    move.b  #$84, PSG_PORT         ; Tone Ch 0
    move.b  #$06, PSG_PORT
    move.b  #$90, PSG_PORT         ; Max Volume
    move.w  #8, Audio_Timer        ; Silence in 8 frames (~130ms)
    rts

Play_Hit_SFX:
    ; Short white noise burst on Channel 3
    move.b  #$E5, PSG_PORT
    move.b  #$F2, PSG_PORT
    move.w  #6, Audio_Timer
    rts

Play_Kill_SFX:
    ; Low thud on Channel 1
    move.b  #$A8, PSG_PORT
    move.b  #$18, PSG_PORT
    move.b  #$B0, PSG_PORT
    move.w  #12, Audio_Timer
    rts

Play_Potion_SFX:
    ; Crystal chime on Channel 2
    move.b  #$C4, PSG_PORT
    move.b  #$02, PSG_PORT
    move.b  #$D0, PSG_PORT
    move.w  #10, Audio_Timer
    rts

Play_PlayerHurt_SFX:
    move.b  #$E6, PSG_PORT
    move.b  #$F2, PSG_PORT
    move.w  #7, Audio_Timer
    rts

Play_LevelClear_SFX:
    move.b  #$88, PSG_PORT
    move.b  #$03, PSG_PORT
    move.b  #$90, PSG_PORT
    move.w  #20, Audio_Timer
    rts

DefaultHandler:
    rte

; ================================================================
; 16. VDP INITIALIZATION TABLE
; ================================================================
VDP_Init_Data:
    dc.w    $8004           ; Reg 0: Colors ON
    dc.w    $8134           ; Reg 1: Display ON, DMA ON, Mode 5, 224 lines
    dc.w    $8230           ; Reg 2: Plane A at VRAM $C000 ($30 * $400)
    dc.w    $832C           ; Reg 3: Window at VRAM $B000 ($2C * $400)
    dc.w    $8407           ; Reg 4: Plane B at VRAM $E000 ($07 * $2000)
    dc.w    $8578           ; Reg 5: Sprite Table at VRAM $F000 ($78 * $200)
    dc.w    $8600           ; Reg 6: Unused
    dc.w    $8700           ; Reg 7: Background color = Pal 0, Color 0
    dc.w    $8800           ; Reg 8: Unused
    dc.w    $8900           ; Reg 9: Unused
    dc.w    $8A00           ; Reg 10: HBlank counter = 0
    dc.w    $8B00           ; Reg 11: Full screen scroll
    dc.w    $8C81           ; Reg 12: 40 cells wide (320 px)
    dc.w    $8D3F           ; Reg 13: HScroll Table at VRAM $FC00 ($3F * $400)
    dc.w    $8E00           ; Reg 14: Unused
    dc.w    $8F02           ; Reg 15: VDP Auto-increment = 2 bytes
    dc.w    $9001           ; Reg 16: Scroll Plane size = 64x32 tiles
    dc.w    $9100           ; Reg 17: Window H = 0
    dc.w    $9200           ; Reg 18: Window V = 0
VDP_Init_Count equ (*-VDP_Init_Data)/2

; ================================================================
; 17. INCLUDED GRAPHICS, PALETTES & MAPS
; ================================================================
    include "C:\Projects\DevilutionX\nightmare_sega\assets\palettes.inc"
    include "C:\Projects\DevilutionX\nightmare_sega\assets\tiles.inc"
    include "C:\Projects\DevilutionX\nightmare_sega\assets\maps.inc"

; Pad ROM to exactly 256 KB ($00040000)
    ds.b    $00040000-*
ROM_End:
