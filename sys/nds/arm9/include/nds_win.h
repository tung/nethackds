#ifndef _NDS_WIN_H_
#define _NDS_WIN_H_

#include "font-bdf.h"
#include "nds_charbuf.h"

#define MAX_WINDOWS 8

#define C_YOU       16 
#define C_MON       17
#define C_PET       18
#define C_WALL      19
#define C_DOOR      20
#define C_ROOM      21
#define C_STAIRS    22
#define C_ALTAR     23
#define C_FURNITURE 24
#define C_CORR      25

typedef struct {
  ANY_P id;
  int count;
  int selected;
  int highlighted;
  int refresh;
  int x, y;
  int width, height;
  char title[96];
} nds_menuitem_t;

typedef struct {
  nds_menuitem_t *items;
  char *prompt;
  u16 count;
  int how;
  int focused_item;
  int tapped_item;
} nds_menu_t;

typedef struct {
  int glyphs[ROWNO][COLNO];
  int cx, cy;
} nds_map_t;

typedef struct {
  u8 type;
  int x, y;
  int width, height;
  int topidx, bottomidx;
  int pagesize;

  // Window type-specific structures.

  nds_charbuf_t *buffer;
  nds_menu_t *menu;
  nds_map_t *map;

  u8 dirty;
  struct ppm *img; 
} nds_nhwindow_t;

extern nds_nhwindow_t *windows[MAX_WINDOWS];
extern struct font *system_font;

void nds_draw_prompt(char *prompt);
void nds_clear_prompt();

char nds_prompt_char(const char *ques, const char *choices, int holdkey);

#endif
