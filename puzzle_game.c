#include <stdlib.h>
#include <string.h>
#include "neslib.h"
#include <nes.h>
// link the pattern table into CHR ROM
//#link "chr_generic.s"
#include "bcd.h"
//#link "bcd.c"
#include "vrambuf.h"
//#link "vrambuf.c"
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

DEF_METASPRITE_2x2(playerRStand, 0xd8, 0);
DEF_METASPRITE_2x2(playerRRun1, 0xdc, 0);
DEF_METASPRITE_2x2(playerRRun2, 0xe0, 0);
DEF_METASPRITE_2x2(playerRRun3, 0xe4, 0);

DEF_METASPRITE_2x2_FLIP(playerLStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun3, 0xe4, 0);

DEF_METASPRITE_2x2(personToSave, 0xba, 1);


const unsigned char* const playerRunSeq[16] = {
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2  
};
// number of actors (4 h/w sprites each)
#define NUM_ACTORS 2

// actor x/y positions
byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
// actor x/y deltas per frame (signed)
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];
/*{pal:"nes",layout:"nes"}*/
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


// setup PPU and tables
void setup_graphics() 
{
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
  setup_graphics();
  // infinite loop
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
