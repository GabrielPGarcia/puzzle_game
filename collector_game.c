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
//------------------------Music-------- 
//#link "music_dangerstreets.s"  
extern char danger_streets_music_data[];  
//#link "demosounds.s"  
extern char demo_sounds[]; 

//--------------------Level-------- 
#include "level_1.h"
#include "level_3.h"
#include "level_4.h"
#include "level_5.h"


//------------------------Player--------
#define NUM_ACTORS 2
#define NUM_POINTS 4
#define NUM_ENEMY 2
#define FP_BITS  4
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



//-----------------Player's sprite--------
DEF_METASPRITE_2x2(playerRStand, 0xd8, 0);
DEF_METASPRITE_2x2(playerRRun1, 0xdc, 0);
DEF_METASPRITE_2x2(playerRRun2, 0xe0, 0);
DEF_METASPRITE_2x2(playerRRun3, 0xe4, 0);

DEF_METASPRITE_2x2_FLIP(playerLStand, 0xd8, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun1, 0xdc, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun2, 0xe0, 0);
DEF_METASPRITE_2x2_FLIP(playerLRun3, 0xe4, 0);

DEF_METASPRITE_2x2(personToSave, 0xba, 1);

const unsigned char* const playerRunSeq[16] = 
{
  playerLRun1, playerLRun2, playerLRun3, playerLRun1, playerLRun2, playerLRun3, 
  playerLRun1, playerLRun2, playerRRun1, playerRRun2, playerRRun3, 
  playerRRun1, playerRRun2, playerRRun3, playerRRun1, playerRRun2  
  };
//-----------------Player's sprite--------
DEF_METASPRITE_2x2(enemyRRun1, 0xdc, 1);
DEF_METASPRITE_2x2(enemyRRun2, 0xe0, 1);
DEF_METASPRITE_2x2(enemyRRun3, 0xe4, 1);
DEF_METASPRITE_2x2_FLIP(enemyLRun1, 0xdc, 1);
DEF_METASPRITE_2x2_FLIP(enemyLRun2, 0xe0, 1);
DEF_METASPRITE_2x2_FLIP(enemyLRun3, 0xe4, 1);


const unsigned char* const enemyRunSeq[16] = 
{
  enemyLRun1, enemyLRun2, enemyLRun3, enemyLRun1, enemyLRun2, enemyLRun3, 
  enemyLRun1, enemyLRun2, enemyRRun1, enemyRRun2, enemyRRun3, 
  enemyRRun1, enemyRRun2, enemyRRun3, enemyRRun1, enemyRRun2  
  };

//---------------------Player Actor--------


const char PALETTE[32] = { 
  0x99,			// screen color

  0x0f,0x21,0x31,0xf,	// background palette 0
  0x00,0x00,0x00,0x0,	// background palette 1
  0x00,0x00,0x00,0x0,	// background palette 2
  0x00,0x00,0x00,0x0,   // background palette 3

  0x0f,0x1c,0x30,0x0,	// sprite palette 0
  0x0f,0xc1,0x30,0x0,	// sprite palette 0
};

byte actor_x[NUM_ACTORS];
byte actor_y[NUM_ACTORS];
sbyte actor_dx[NUM_ACTORS];
sbyte actor_dy[NUM_ACTORS];

byte enemy_x[NUM_ENEMY];
byte enemy_y[NUM_ENEMY];
sbyte enemy_dx[NUM_ENEMY];
sbyte enemy_dy[NUM_ENEMY];


typedef struct Actor {
  word yy;		// Y position in pixels (16 bit)
  byte x;		// X position in pixels (8 bit)
  byte y;		// X position in pixels (8 bit)
  byte state;		// ActorState
  //int dir:1;		// direction (0=right, 1=left)
  int onscreen:1;	// is actor onscreen?
} Actor;
typedef struct Points {
  byte x;		// X position in pixels (8 bit)
  byte y;		// X position in pixels (8 bit)
  byte state;		// ActorState
} Points;
Points points[4];

Actor actors;	// all actors
Actor enemy[3];	// all actors

//---------------------Player Actor--------
char enemyt1;
char enemyt2;
int score = 00;
void setup_graphics() 
{
  ppu_off();
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
  // unpack nametable into the VRAM
  vram_adr(0x2000);
  //vram_unrle(rle);
  ppu_on_all();
}
byte rndint(byte a, byte b)
{
  return(rand()%(b-a))+a;
}
void points_cllector(int i)
{
  if((points[i].x >= actor_x[0]-4 && points[i].x <= actor_x[0]+8)&& (points[i].y >= actor_x[0]-2 && points[i].y<= actor_x[0]+4))
  {

    enemy_dx[i] = - enemy_dx[i];
    enemy_dx[0] = - enemy_dx[i];
    enemy_dy[i] = - enemy_dy[i];
    score = score + 1;
  }
  if(points[i].state == 0 && points[i].y <= 160 && points[i].y >= 130)
  {
    points[i].x = rndint(45,230);
    points[i].y = rndint(10,50);
    points[i].state = 18;
  }
};


void player_action(char i,char pad, char oam_id, int iRand)
{ 
  oam_id = 0;
  iRand = (rand()%3)+2;

    points_cllector(i);
  for (i=0; i<3; i++) {
    // poll controller i (0-1)
    pad = pad_trigger(0);
    // move actor[i] left/right
    if (pad&PAD_LEFT && actor_x[i]+1>24){ actor_dx[i]=-3;}
    else if (pad&PAD_RIGHT && actor_x[i]+1<216){ actor_dx[i]=3;}
    // move actor[i] up/down
    else if (pad&PAD_UP && actor_y[i]+1>47) actor_dy[i]=-2;
    else if (pad&PAD_DOWN && actor_y[i]+1<191) actor_dy[i]=2;
  }
  if(actor_x[0] >= 24 &&  actor_x[0] != 216 )
  {
    actor_dx[0] = - actor_dx[0];
  }
  if(actor_x[0] <= 216 && actor_x[0] != 24 )
  {
    actor_dx[0] = - actor_dx[0];
  }  
  if(actor_y[0] >= 47 &&  actor_y[0] != 191)
  {
    actor_dy[0] = - actor_dy[0];  
  }
  if(actor_y[0] <= 191 &&  actor_y[0] != 47)
  {
    actor_dy[0] = - actor_dy[0];
  } 
  for (i=0; i<NUM_ACTORS; i++) {
    byte runseq = actor_x[i] & 7;
    if (actor_dx[i] >= 0 )
      runseq += 8;
    oam_id = oam_meta_spr(actor_x[i], actor_y[i], oam_id, playerRunSeq[runseq]);
    actor_x[i] += actor_dx[i];
    actor_y[i] += actor_dy[i];
  }
  if (oam_id!=0) oam_hide_rest(oam_id);


  oam_id = oam_spr(points[i].x, points[i].y, points[i].state, 1, oam_id);

  //Draws and updates Scoreboard
  oam_id = oam_spr(63, 15, (score/10%10)+48, 4, oam_id);
  oam_id = oam_spr(69, 15, (score%10)+48, 4, oam_id);
  //points_cllector(i);
  for (i=0; i<2; i++) 
  {
    if((enemy_x[0] >= enemy_x[1]-8 && enemy_x[0] <= enemy_x[1]+8)&& (enemy_y[0] >= enemy_y[1]-8 && enemy_y[0] <= enemy_y[1]+8))
    {

      enemy_dx[i] = - enemy_dx[i];
      enemy_dx[0] = - enemy_dx[i]+(iRand-1);
      enemy_dy[i] = - enemy_dy[i];
      score = enemy_dx[0] ;
    }
    if(enemy_x[i] > 24 &&  enemy_x[i] != 216)
    {
      enemy_dx[i] = -enemy_dx[i];
    }
    if(enemy_x[i] < 216 && enemy_x[i] != 24)
    {
      enemy_dx[i] = - enemy_dx[i];
    }  
    if(enemy_y[i] > 47&&  enemy_y[i] != 191)
    {
      enemy_dy[i] = - enemy_dy[i];  
    }
    if(enemy_y[i] < 191&&  enemy_y[i] != 47)
    {
      enemy_dy[i] = - enemy_dy[i];
    }  
  }
  for (i=0; i<NUM_ACTORS; i++) {
    byte runseq = enemy_x[i] & 7;
    if (enemy_dx[i] >= 0 )
      runseq += 8;

    oam_id = oam_meta_spr(enemy_x[i], enemy_y[i],oam_id,enemyRunSeq[runseq]);
    enemy_x[i] += enemy_dx[i];
    enemy_y[i] += enemy_dy[i];
  }
  if (oam_id!=0) oam_hide_rest(oam_id);
  // wait for next frame
  ppu_wait_frame();

}

void main(void)
{
  char i = 0;
  int k=0;
  char pad;
  char oam_id;

  int iRand = (rand()%2);

  actor_x[0] = 125;
  actor_y[0] = 100;
  actor_dx[0] = 1;
  actor_dy[0] = 1;
  actor_x[1] = 125;
  actor_y[1] = -20;
  for(i = 0; i <2; i++)
  {
    iRand = (rand()%5);
    enemy_x[i] = 50*(i+1);
    enemy_y[i] = 180*(i+1);
    enemy_dx[i] = iRand - 1;
    enemy_dy[i] = iRand - 1;     
  } 
  for(i = 0; i <3; i++)
  {
    points[i].x = rndint(50,230);
    points[i].y = rndint(50,70);
    points[i].state = 175;       
  }
  ppu_off();
  //pal_bg(level_1);
  //vram_adr(0x2000);
  vram_unrle(level_1);

  vram_adr(NTADR_A(1,2)); // set address
  vram_write("Points:", 7);
  famitone_init(danger_streets_music_data);
  //sfx_init(demo_sounds);
  nmi_set_callback(famitone_update);
  music_play(0);
  //sfx_play(0,1);

  setup_graphics();

  while(1)
  {
    
    player_action(i,pad,oam_id,iRand);
  }
}