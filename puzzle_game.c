#include <stdlib.h>
#include <string.h>
#include "neslib.h"
#include <nes.h>
//#link "chr_generic.s"
#include "bcd.h"
//#link "bcd.c"
// VRAM update buffer
#include "vrambuf.h"
//#link "vrambuf.c"

//#link "famitone2.s"
void __fastcall__ famitone_update(void);
//#link "music_aftertherain.s"
extern char after_the_rain_music_data[]; 

#include "level_1.h"
#include "level_2.h"
#include "level_3.h"
#include "level_4.h"
#include "level_5.h"

//game uses 12:4 fixed point calculations for enemy movements

#define FP_BITS  4

//max size of the game map

#define MAP_WDT      16
#define MAP_WDT_BIT    4
#define MAP_HGT      16


#define TILE_SIZE    16
#define TILE_SIZE_BIT  4


#define LEVELS_ALL    5

// number of actors (4 h/w sprites each)
#define NUM_ACTORS 2

#define DEF_METASPRITE_2x2(name,code,pal)\
const unsigned char name[]={\
  0,0, (code)+0, pal, \
  0,8, (code)+1, pal, \
  8,0, (code)+2, pal, \
  8,8, (code)+3, pal,\
  128\
};
#define DEF_METASPRITE_2x2_FLIP(name,code,pal)\
const unsigned char name[]={\
        8,      0,      (code)+0,   (pal)|OAM_FLIP_H, \
        8,      8,      (code)+1,   (pal)|OAM_FLIP_H, \
        0,      0,      (code)+2,   (pal)|OAM_FLIP_H, \
        0,      8,      (code)+3,   (pal)|OAM_FLIP_H, \
        128};

 
//#link "music_dangerstreets.s"  
extern char danger_streets_music_data[];  
//#link "demosounds.s"  
extern char demo_sounds[]; 




DEF_METASPRITE_2x2(playerRStand, 0xd8, 0);
DEF_METASPRITE_2x2(playerRRun1, 0xdc, 0);
DEF_METASPRITE_2x2(playerRRun2, 0xe0, 0);
DEF_METASPRITE_2x2(playerRRun3, 0xe4, 0);

DEF_METASPRITE_2x2_FLIP(playerLStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun3, 0xe4, 0);

DEF_METASPRITE_2x2(personToSave, 0xba, 1);
///// ACTORS

typedef enum ActorState {
  INACTIVE, STANDING, WALKING, CLIMBING, JUMPING, FALLING, PACING
};

typedef enum ActorType {
  ACTOR_PLAYER, ACTOR_ENEMY, ACTOR_RESCUE
};

const unsigned char* const playerRunSeq[16] = {
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2  
};
// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];

const char PALETTE[32] = { 
  0x03,			// screen color

  0x11,0x30,0x27,0x0,	// background palette 0
  0x1c,0x20,0x2c,0x0,	// background palette 1
  0x00,0x10,0x20,0x0,	// background palette 2
  0x06,0x16,0x26,0x0,   // background palette 3

  0x16,0x35,0x24,0x0,	// sprite palette 0
  0x00,0x37,0x25,0x0,	// sprite palette 1
  0x0d,0x2d,0x3a,0x0,	// sprite palette 2
  0x0d,0x27,0x2a	// sprite palette 3
};
//palettes data
//all the palettes are designed in NES Screen Tool, then copy/pasted here
//with Palettes/Put C data to clipboard function
/*{pal:"nes",layout:"nes"}*/
const unsigned char palGame1[16]={ 
  0x0f,0x11,0x32,0x30,
  0x0f,0x1c,0x2c,0x3c,
  0x0f,0x09,0x27,0x38,
  0x0f,0x11,0x21,0x31 };
/*{pal:"nes",layout:"nes"}*/
const unsigned char palGame2[16]={ 
  0x0f,0x11,0x32,0x30,
  0x0f,0x11,0x21,0x31,
  0x0f,0x07,0x27,0x38,
  0x0f,0x13,0x23,0x33 };
/*{pal:"nes",layout:"nes"}*/
const unsigned char palGame3[16]={ 
  0x0f,0x11,0x32,0x30,
  0x0f,0x15,0x25,0x35,
  0x0f,0x05,0x27,0x38,
  0x0f,0x13,0x23,0x33 };
/*{pal:"nes",layout:"nes"}*/
const unsigned char palGame4[16]={ 
  0x0f,0x11,0x32,0x30,
  0x0f,0x19,0x29,0x39,
  0x0f,0x0b,0x27,0x38,
  0x0f,0x17,0x27,0x37 };
/*{pal:"nes",layout:"nes"}*/
const unsigned char palGame5[16]={ 
  0x0f,0x11,0x32,0x30,
  0x0f,0x16,0x26,0x36,
  0x0f,0x07,0x27,0x38,
  0x0f,0x18,0x28,0x38 };
/*{pal:"nes",layout:"nes"}*/
const unsigned char palGameSpr[16]={ 
  0x0f,0x0f,0x29,0x30,
  0x0f,0x0f,0x26,0x30,
  0x0f,0x0f,0x24,0x30,
  0x0f,0x0f,0x21,0x30 };
/*{pal:"nes",layout:"nes"}*/
const unsigned char palTitle[16]={ 
  0x0f,0x0f,0x0f,0x0f,
  0x0f,0x1c,0x2c,0x3c,
  0x0f,0x12,0x22,0x32,
  0x0f,0x14,0x24,0x34 };

typedef struct Actor {
  word yy;		// Y position in pixels (16 bit)
  byte x;		// X position in pixels (8 bit)
  byte y;		// X position in pixels (8 bit)
  byte floor;		// floor index
  byte state;		// ActorState
  sbyte yvel;		// Y velocity (when jumping)
  sbyte xvel;		// X velocity (when jumping)
  int name:2;		// ActorType (2 bits)
  int pal:2;		// palette color (2 bits)
  int dir:1;		// direction (0=right, 1=left)
  int onscreen:1;	// is actor onscreen?
} Actor;

Actor actors[1];	// all actors

void move_actor(struct Actor* actor, byte joystick) {
  switch (actor->state) {
      
    case STANDING:
    case WALKING:
      // left/right has priority over climbing
      if (joystick & PAD_UP) {
        actor->x--;
        actor->dir = 1;
        actor->state = WALKING;
      } 
      else if (joystick & PAD_LEFT) {
        actor->x--;
        actor->dir = 1;
        actor->state = WALKING;
      } 
      else if (joystick & PAD_RIGHT) {
        actor->x++;
        actor->dir = 0;
        actor->state = WALKING;
      } 
      else {
        actor->state = STANDING;
      }
      break;
  }

}
void setup_graphics() 
{
  ppu_off();
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
  ppu_on_all();
}




void main(void)
{
  int i;
  char pad;
  char oam_id;
  int lastAct = 0;
  actor_x[0] = 125;
  actor_y[0] = 100;
  actor_dx[0] = 0;
  actor_dy[0] = 0;
  
  actor_x[1] = 45;
  actor_y[1] = 9;
  actor_dx[1] = 0;
  actor_dy[1] = 0;
  
  vram_adr(NTADR_A(1,2));		// set address
  vram_write("life:", 5);
  famitone_init(danger_streets_music_data);
  //sfx_init(demo_sounds);
  nmi_set_callback(famitone_update);
  music_play(0);
//sfx_play(0,1);
  setup_graphics();
  
  while(1)
  {
        oam_id = 0;
    
     for (i=0; i<2; i++) {
      // poll controller i (0-1)
      pad = pad_poll(i);
      // move actor[i] left/right
      if (pad&PAD_LEFT && actor_x[i]>3){ actor_dx[i]=-2;lastAct = 0;}
      else if (pad&PAD_RIGHT && actor_x[i]<236){ actor_dx[i]=2;lastAct = 1;}
      else actor_dx[i]=0;
      // move actor[i] up/down
      if (pad&PAD_UP && actor_y[i]>3) actor_dy[i]=-2;
      else if (pad&PAD_DOWN && actor_y[i]<206) actor_dy[i]=2;
      else actor_dy[i]=0;
    }
    
    for (i=0; i<NUM_ACTORS; i++) {
      byte runseq = actor_x[i] & 7;
      if (actor_dx[i] >= 0 & lastAct == 1)
        runseq += 8;
      oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[runseq]);
      actor_x[i] += actor_dx[i];
      actor_y[i] += actor_dy[i];
    }
  if (oam_id!=0) oam_hide_rest(oam_id);
    // wait for next frame
    ppu_wait_frame();


  }
}