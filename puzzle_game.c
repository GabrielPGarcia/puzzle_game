
#include "Title.h"
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

//------------------------Player--------
#define NUM_ACTORS 2
#define NUM_ENEMY 2

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
DEF_METASPRITE_2x2(enemyRStand, 0xd8, 5);
DEF_METASPRITE_2x2(enemyRRun1, 0xdc, 5);
DEF_METASPRITE_2x2(enemyRRun2, 0xe0, 5);
DEF_METASPRITE_2x2(enemyRRun3, 0xe4, 5);

DEF_METASPRITE_2x2_FLIP(enemyLStand, 0x8b, 5);
DEF_METASPRITE_2x2_FLIP(enemyLRun1, 0xdc, 5);
DEF_METASPRITE_2x2_FLIP(enemyLRun2, 0xe0, 5);
DEF_METASPRITE_2x2_FLIP(enemyLRun3, 0xe4, 5);
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

Points points[3];
Actor actors;	// all actors
Actor enemy[4];	// all actors
int playerp;
int playerl;
char i;
char pad;
char oam_id;
int iRand;
int j ;
int k = 0;
//---------------------Player Actor--------

void setup_graphics() 
{
  ppu_off();
  // clear sprites
  oam_clear();
  // set palette colors
  pal_all(PALETTE);
  ppu_on_all();
}
void actors_setup(int iRand)
{
  actor_x[0] = 125;
  actor_y[0] = 100;
  actor_dx[0] = 1;
  actor_dy[0] = 1;
  actor_x[1] = 125;
  actor_y[1] = -20;
  for(i = 0; i < 2; i++)
  {
    iRand=(rand()%5);
    enemy_x[i] = 50+iRand+55;
    enemy_y[i] = 50+iRand+24;
    enemy_dx[i] = -3;
    enemy_dy[i] = 2;

  }
}
void point_to_lives()
{
  if(playerp/10%10 == 5 && j == 0)
  {
    playerl = playerl+1;
    j==1;
  }
  else if(playerp/10%10 == 0 && j == 1)
  { 
    playerl = playerl-1;
    j==0;
  }
  if(playerl >= 9)
    playerl = 9;
}
void pionts_action()
{  
  for(i = 0; i<3; i++)
  {
    if((points[i].x >= actor_x[0]-4 && points[i].x <= actor_x[0]+8)&& 
       (points[i].y >= actor_y[0]-2 && points[i].y <= actor_y[0]+4)) 
    {
      points[i].state = 0;
      playerp = playerp + 1;
    }

    if(points[i].state == 0 )
    {    
      points[i].x = (rand()%(35-200))+35;
      points[i].y = (rand()%(55-180))+55;    
      points[i].state = 175;
    }
    if(points[i].state == 175)

      oam_id = oam_spr(points[i].x, points[i].y, points[i].state, 4, oam_id);
 }
  
  if(playerp<=0)playerp=0;
  if(playerp >=100)oam_id = oam_spr(63, 15, (playerp/100%10)+48, 4, oam_id);
  if(playerp >=10)oam_id = oam_spr(69, 15, (playerp/10%10)+48, 4, oam_id);
  oam_id = oam_spr(75, 15, (playerp%10)+48, 4, oam_id);
  oam_id = oam_spr(207, 15, (playerl%10)+48, 4, oam_id);

}
void enemys_action()
{
  iRand = (rand()%(2+playerp/100%10));
  for (i=0; i<2; i++) 
  {
    if((enemy_x[0] >= enemy_x[1]-2 && enemy_x[0] <= enemy_x[1]+2) && 
       (enemy_y[0] >= enemy_y[1]-2 && enemy_y[0] <= enemy_y[1]+2))
    {
      enemy_dx[i] = - enemy_dx[i];
      enemy_dx[0] = - enemy_dx[i]-iRand;
      enemy_dy[i] = - enemy_dy[i];        
    }
    //-------------wall collision----------

    if(enemy_x[i] > 24 &&  enemy_x[i] != 216)
    {
      enemy_dx[i] = - enemy_dx[i];
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
  for (i=0; i<NUM_ENEMY; i++) {
    byte runseq = enemy_x[i] & 7;
    if (enemy_dx[i] >= 0 )
      runseq += 8;
    oam_id = oam_meta_spr(enemy_x[i], enemy_y[i], oam_id, enemyRunSeq[runseq]);
    enemy_x[i] += enemy_dx[i];
    enemy_y[i] += enemy_dy[i];
  }
  if (oam_id!=0) oam_hide_rest(oam_id);
}
void player_action()
{ 
  for (i=0; i<3; i++) 
  {
    pad = pad_trigger(0);
    if((actor_x[0]>35 && actor_x[0]<200)&&(actor_y[0]>55 && actor_y[0]<180))
    {
      if(pad&PAD_LEFT)actor_dx[0]=-3-(playerp/100%10);
      else if (pad&PAD_RIGHT)actor_dx[0]=3+(playerp/100%10);
      else if (pad&PAD_UP)actor_dy[0]=-2-(playerp/100%10);
      else if (pad&PAD_DOWN)actor_dy[0]=2+(playerp/100%10);
    } 
  }
  //-------------wall collision----------
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
}
void player_enemy_collision()
{
  for (i=0; i<3; i++) 
  {
    if((actor_x[0]+4 >= enemy_x[i]-4 && actor_x[0]-4 <= enemy_x[i]+4) && 
       (actor_y[0]+4 >= enemy_y[i]-4 && actor_y[0]-4 <= enemy_y[i]+4))
    {
      actors_setup(iRand); 
      playerl = playerl-1;
    }
  }
}

void game_loop()
{ 
  
  pionts_action();
  point_to_lives();
  iRand = (rand()%4);


  player_action();
  player_enemy_collision();
  enemys_action();
  
  ppu_wait_frame();

}

void main(void)
{
  iRand = (rand()%2);
  j=0;
  playerp = 0;
  playerl = 3;
  i = 0;
  actors_setup(iRand);
  for(i = 0; i <3; i++)
  {
    points[i].x = (rand()%(35-200))+35;
    points[i].y = (rand()%(55-180))+55;    
    points[i].state = 175;
  }
  ppu_off();
  vram_unrle(level_1);

  vram_adr(NTADR_A(1,2)); // set address
  vram_write("Points:", 7);  
  vram_adr(NTADR_A(20,2)); // set address
  vram_write("Lives:", 6);
  famitone_init(danger_streets_music_data);
  //sfx_init(demo_sounds);
  nmi_set_callback(famitone_update);
  music_play(0);
  //sfx_play(0,1);

  setup_graphics();

  while(1)
  {
    oam_id = 0;
    game_loop();
  }
}