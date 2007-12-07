#include <nds.h>
#include <stdio.h>

#include "hack.h"
#include "func_tab.h"

#include "nds_win.h"
#include "nds_cmd.h"
#include "nds_gfx.h"
#include "ppm-lite.h"
#include "nds_util.h"
#include "nds_map.h"
#include "ds_kbd.h"

#define M(c) (0x80 | (c))
#define C(c) (0x1f & (c))

#define COLWIDTH 60

#define KEY_CONFIG_FILE "keys.cnf"

#define CMD_CONFIG     0x0100

/*
 * Missing commands:
 *
 * conduct
 * ride
 * extended commands
 */

typedef struct {
  u16 f_char;
  char *name;
  int x1;
  int y1;
  int x2;
  int y2;
} nds_cmd_t;

typedef struct {
  int key;
  char *name;
} nds_key_t;

static nds_cmd_t cmdlist[] = {
//	{C('p'), TRUE, doprev_message},
	{M('a'), "Adjust"},
	{'a', "Apply"},
	{'A', "Armor"},
	{C('x'), "Attributes"},
	{'C', "Call"},
	{'Z', "Cast"},
	{M('c'), "Chat"},
	{'c', "Close"},
	{M('d'), "Dip"},
	{'\\', "Discoveries"},	
	{'>', "Down"},
	{'q', "Drink"},
	{'D', "Drop"},
	{'e', "Eat"},
	{'E', "Engrave"},
	{M('e'), "Enhance"},
        {'#', "Ex-Cmd"},
	{'X', "Explore"},
	{'f', "Fire"},
	{M('f'), "Force"},
	{'?', "Help"},
	{'V', "History"},
	{'^', "Id"},
	{'*', "In Use"},
	{'i', "Inventory"},
	{M('i'), "Invoke"},
	{M('j'), "Jump"},
        {CMD_CONFIG, "Key Config"},
	{C('d'), "Kick"},
	{':', "Look"},
	{M('l'), "Loot"},
	{M('m'), "Monster"},
	{M('n'), "Name"},
	{'o', "Open"},
	{'p', "Pay"},
	{',', "Pickup"},
	{M('p'), "Pray"},
	{'P', "Put On"},
	{'Q', "Quiver"},
	{'r', "Read"},
        {'\001', "Redo"},
	{'R', "Remove"},
	{M('r'), "Rub"},
	{M('o'), "Sacrifice"},
	{'S', "Save"},
	{'s', "Search"},
	{'O', "Set"},
	{M('s'), "Sit"},
	{'x', "Swap"},
	{'T', "Take Off"},
	{C('t'), "Teleport"},
	{'t', "Throw"},
//	{'@', "Toggle Pickup"},
	{M('2'), "Two Weapon"},
	{M('t'), "Turn"},
	{'I', "Type-Inv"},
	{'<', "Up"},
	{M('u'), "Untrap"},
//	{'v', "Version"},
	{'.', "Wait"},
	{'&', "What Does"},
	{';', "What Is"},
	{'w', "Wield"},
	{'W', "Wear"},
	{M('w'), "Wipe"},
	{'z', "Zap"},
//	{'/', "What Is"},
        /*
	{WEAPON_SYM,  TRUE, doprwep},
	{ARMOR_SYM,  TRUE, doprarm},
	{RING_SYM,  TRUE, doprring},
	{AMULET_SYM, TRUE, dopramulet},
	{TOOL_SYM, TRUE, doprtool},
        */
        /*
	{GOLD_SYM, TRUE, doprgold},
	{SPBOOK_SYM, TRUE, dovspell},
        */
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL},
        {0, NULL}
};


static nds_cmd_t wiz_cmdlist[] = {
#ifdef WIZARD
	{C('e'), "Wiz-Detect"},
	{C('g'), "Wiz-Genesis"},
	{C('i'), "Wiz-Identify"},
	{C('f'), "Wiz-Map"},
	{C('v'), "Wiz-Tele"},
	{C('o'), "Wiz-Where"},
	{C('w'), "Wiz-Wish"},
#endif
        {0, NULL}
};

/* We use this array for indexing into the key config list */

static nds_key_t keys[] = {
  { KEY_A, "A" },
  { KEY_B, "B" },
  { KEY_X, "X" },
  { KEY_Y, "Y" },
  { KEY_SELECT, "Select" },
  { KEY_START, "Start" },
  { KEY_RIGHT, "Right" },
  { KEY_LEFT, "Left" },
  { KEY_UP, "Up" },
  { KEY_DOWN, "Down" },
  { -1, NULL }
};

u16 key_map[] = {
  ',', 's', 'o', C('d'),
  0, 0,
  'l', 'h', 'k', 'j'
};

u16 *vram = (u16 *)BG_BMP_RAM(12);

u16 cmd_key = KEY_L;
u16 scroll_key = KEY_R;

nds_cmd_t nds_cmd_loop();
nds_cmd_t nds_kbd_cmd_loop();
void nds_load_key_config();

void nds_init_cmd()
{
  int cur_x = 0;
  int cur_y = 0;
  int i;
  struct ppm *img = alloc_ppm(256, 192);

  if (flags.debug) {
    int idx = 0;

    for (; cmdlist[idx].name != NULL; idx++);

    for (i = 0; wiz_cmdlist[i].name != NULL; i++) {
      cmdlist[idx++] = wiz_cmdlist[i];
    }
  }

  for (i = 0; cmdlist[i].name != NULL; i++) {
    int text_h;

    text_dims(system_font, cmdlist[i].name, NULL, &text_h);

    if ((cur_y + text_h) > 192) {
      cur_x += COLWIDTH + 1;
      cur_y = 0;
    }

    draw_string(system_font, cmdlist[i].name, img,
                cur_x, cur_y, 1,
                255, 0, 255);

    cmdlist[i].x1 = cur_x;
    cmdlist[i].x2 = cur_x + COLWIDTH;
    cmdlist[i].y1 = cur_y;
    cmdlist[i].y2 = cur_y + text_h;

    if (flags.debug) {
      cur_y += text_h;
    } else {
      cur_y += text_h + 2;
    }
  }

  draw_ppm_bw(img, vram, 0, 0, 256, 254, 255);

  free_ppm(img);

  nds_load_key_config();

  if (iflags.lefthanded) {
    cmd_key = KEY_R;
    scroll_key = KEY_L;
  }
}

int nds_map_key(u16 pressed)
{
  int i;

  for (i = 0; keys[i].key > 0; i++) {
    if (pressed & keys[i].key) {
      return key_map[i];
    }
  }
  
  return 0;
}

void nds_load_key_config()
{
  FILE *fp = fopen(fqname(KEY_CONFIG_FILE, CONFIGPREFIX, 0), "r");
  u8 buffer[BUFSZ];
  int cnt = sizeof(key_map);
  int ret;

  if (fp == (FILE *)0) {
    return;
  }

  if ((ret = fread(buffer, 1, cnt, fp)) < cnt) {
    iprintf("Only got %d, wanted %d\n", ret, cnt);
    return;
  } 
  
  memcpy(key_map, buffer, cnt);

  fclose(fp);
}

void nds_save_key_config()
{
  FILE *fp = fopen(fqname(KEY_CONFIG_FILE, CONFIGPREFIX, 0), "w");

  if (fp == (FILE *)0) {
    return;
  }

  fwrite(key_map, 1, sizeof(key_map), fp);

  fclose(fp);
}

nds_cmd_t nds_get_config_cmd()
{
  winid win;
  menu_item *sel;
  ANY_P ids[6];
  nds_cmd_t cmd;

  win = create_nhwindow(NHW_MENU);
  start_menu(win);

  ids[0].a_int = 0;
  ids[1].a_int = 'k';
  ids[2].a_int = 'j';
  ids[3].a_int = 'h';
  ids[4].a_int = 'l';
  ids[5].a_int = 1;

  add_menu(win, NO_GLYPH, &(ids[0]), 0, 0, 0, "Direction Keys", 0);

  add_menu(win, NO_GLYPH, &(ids[1]), 0, 0, 0, "Up", 0);
  add_menu(win, NO_GLYPH, &(ids[2]), 0, 0, 0, "Down", 0);
  add_menu(win, NO_GLYPH, &(ids[3]), 0, 0, 0, "Left", 0);
  add_menu(win, NO_GLYPH, &(ids[4]), 0, 0, 0, "Right", 0);

  add_menu(win, NO_GLYPH, &(ids[0]), 0, 0, 0, "Other", 0);

  add_menu(win, NO_GLYPH, &(ids[5]), 0, 0, 0, "Game Command", 0);

  end_menu(win, "What do you want to assign to this key?");

  if (select_menu(win, PICK_ONE, &sel) <= 0) {
    cmd.f_char = 0;
    cmd.name = NULL;
  } else if (sel->item.a_int == 1) {
    cmd = nds_cmd_loop(1);
  } else {
    cmd.f_char = sel->item.a_int;

    switch (cmd.f_char) {
      case 'k':
        cmd.name = "Move Up";
        break;

      case 'j':
        cmd.name = "Move Down";
        break;

      case 'h':
        cmd.name = "Move Left";
        break;

      case 'l':
        cmd.name = "Move Right";
        break;

      default:
        cmd.name = NULL;
        break;
    }
  }

  NULLFREE(sel);

  destroy_nhwindow(win);

  return cmd;
}

void nds_config_key()
{
  int i;
  int pressed;
  nds_key_t key = { 0, NULL };
  nds_cmd_t cmd;
  char buf[BUFSZ];

  nds_draw_prompt("Press the key to modify.");

  while (1) {
    swiWaitForVBlank();

    scanKeys();

    pressed = keysDown();

    /* We don't let the user configure these */

    if ((pressed & KEY_L) ||
        (pressed & KEY_R)) {
      continue;
    } else if (pressed) {
      break;
    }
  }

  nds_clear_prompt();

  for (i = 0; keys[i].key > 0; i++) {
    if (pressed & keys[i].key) {
      key = keys[i];
      break;
    }
  }

  cmd = nds_get_config_cmd();

  if (cmd.name == NULL) {
    return;
  }

  key_map[i] = cmd.f_char;

  sprintf(buf, "Mapped %s to %s.", key.name, cmd.name);

  clear_nhwindow(WIN_MESSAGE);
  putstr(WIN_MESSAGE, ATR_NONE, buf);

  nds_save_key_config();
}

void nds_swap_handedness()
{
  u16 tmp = cmd_key;

  cmd_key = scroll_key;
  scroll_key = tmp;

  nds_save_key_config();

  clear_nhwindow(WIN_MESSAGE);

  if (cmd_key == KEY_L) {
    putstr(WIN_MESSAGE, ATR_NONE, "Switched to right-handed mode.");
  } else {
    putstr(WIN_MESSAGE, ATR_NONE, "Switched to left-handed mode.");
  }
}

int nds_nh_poskey(int *x, int *y, int *mod)
{
  touchPosition coords;

  /* Clear out any taps that happen to be occuring right now. */

  nds_flush();

  while(1) {
    int key = 0;
    int pressed;
    int held;

    scan_touch_screen();
    scanKeys();

    pressed = keysDownRepeat();
    held = keysHeld();

    if (held & scroll_key) {
      int cx, cy;
      int changed = 0;

      nds_map_get_center(&cx, &cy);

      if (pressed & KEY_UP) {
        cy--;
        changed |= 1;
      } 
      
      if (pressed & KEY_DOWN) {
        cy++;
        changed |= 1;
      } 
      
      if (pressed & KEY_LEFT) {
        cx--;
        changed |= 1;
      } 
      
      if (pressed & KEY_RIGHT) {
        cx++;
        changed |= 1;
      }

      if (changed) {
        nds_draw_map(windows[WIN_MAP]->map, &cx, &cy);
      } else {
        swiWaitForVBlank();
      }

      continue;
    }

    swiWaitForVBlank();

    if (pressed & cmd_key) {
      nds_cmd_t cmd;
      
      if (iflags.cmdwindow) {
        cmd = nds_cmd_loop(0);
      } else {
        cmd = nds_kbd_cmd_loop();
      }

      key = cmd.f_char;
    } else if (pressed) {
      key = nds_map_key(pressed);
    }

    switch (key) {
      case 0:
        break;

      case CMD_CONFIG:
        nds_config_key();
        break;

      default:
        return key;
    }

    if (get_touch_coords(&coords)) {
      nds_map_translate_coords(coords.px, coords.py, x, y);

      *mod = CLICK_1;

      return 0;
    }
  }

  return 0;
}

nds_cmd_t nds_find_command(int x, int y)
{
  int i;
  nds_cmd_t cmd = {0, NULL};

  for (i = 0; cmdlist[i].name != NULL; i++) {
    if ((x >= cmdlist[i].x1) && (x <= cmdlist[i].x2) &&
        (y >= cmdlist[i].y1) && (y <= cmdlist[i].y2)) {
      cmd = cmdlist[i];

      break;
    }
  }

  return cmd;
}

nds_cmd_t nds_cmd_loop(int in_config)
{
  nds_cmd_t curcmd = { 0, NULL };
  nds_cmd_t picked_cmd = { 0, NULL };
  u16 old_bg_cr;

  touchPosition coords = { .x = 0, .y = 0 };
  touchPosition lastCoords;

  /* Initialize our display */
  old_bg_cr = BG2_CR;

  BG2_CR = BG_BMP8_256x256 | BG_BMP_BASE(12) | BG_PRIORITY_1;
  DISPLAY_CR |= DISPLAY_BG2_ACTIVE;

  /*
   * Now, we loop until either a command is tapped and selected, or the left
   * button is released.
   */
  while (1) {
    int held;
    int pressed;

    swiWaitForVBlank();

    lastCoords = coords;
    coords = touchReadXY();

    scanKeys();

    pressed = keysDown();
    held = keysHeld();

    if (in_config && (pressed & KEY_B)) {
      break;
    } else if (! in_config && ! (held & cmd_key)) {
      break;
    }

    if ((coords.x != 0) && (coords.y != 0)) {
      nds_cmd_t cmd = nds_find_command(coords.px, coords.py);

      if (cmd.f_char != curcmd.f_char) {
        if (curcmd.name != NULL) {
          nds_draw_text(system_font, curcmd.name, curcmd.x1, curcmd.y1,
                        254, 255, vram);
        }

        if (cmd.name != NULL) {
          nds_draw_text(system_font, cmd.name, cmd.x1, cmd.y1, 
                        254, 253, vram);
        }

        curcmd = cmd;
      }
    } else if ((lastCoords.x != 0) && (lastCoords.y != 0)) {
      nds_cmd_t cmd = nds_find_command(lastCoords.px, lastCoords.py);

      if (cmd.name != NULL) {
        nds_draw_text(system_font, cmd.name, cmd.x1, cmd.y1, 
                      254, 255, vram);

        picked_cmd = cmd;

        break;
      }
    }
  }

  /*
   * This happens if the user releases L while pressing a command.  Basically,
   * we want to remove the item highlight.
   */

  if ((coords.x != 0) && (coords.y != 0)) {
    nds_cmd_t cmd = nds_find_command(coords.px, coords.py);

    if (cmd.name != NULL) {
      nds_draw_text(system_font, cmd.name, cmd.x1, cmd.y1, 
                    254, 255, vram);
    }
  }

  DISPLAY_CR ^= DISPLAY_BG2_ACTIVE;
  BG2_CR = old_bg_cr;

  return picked_cmd;
}

nds_cmd_t nds_kbd_cmd_loop()
{
  nds_cmd_t cmd = { 0, NULL };
  int key;
  int held;
  int done = 0;

  DISPLAY_CR |= DISPLAY_BG0_ACTIVE;

  while (! done) {
    swiWaitForVBlank();
    scanKeys();

    key = kbd_vblank();
    held = keysHeld();

    if (! (held & cmd_key)) {
      goto DONE;
    }

    switch (key) {
      case 0:
      case K_UP_LEFT:
      case K_UP:
      case K_UP_RIGHT:
      case K_NOOP:
      case K_DOWN_LEFT:
      case K_DOWN:
      case K_DOWN_RIGHT:
      case K_LEFT:
      case K_RIGHT:
      case '\n':
      case '\b':
        continue;

      default:
        done = 1;
        break;
    }
  }

  cmd.f_char = key;
  cmd.name = "Dummy";

  while (1) { 
    swiWaitForVBlank();
    scanKeys();
    kbd_vblank();

    if (keysUp() & KEY_TOUCH) {
      break;
    }
  };

DONE:

  DISPLAY_CR ^= DISPLAY_BG0_ACTIVE;

  return cmd;
}

int nds_get_ext_cmd()
{
  char buffer[BUFSZ];
  int i;

  getlin("Extended Command", buffer);

  for (i = 0; extcmdlist[i].ef_txt != NULL; i++) {
    if (strcmp(extcmdlist[i].ef_txt, buffer) == 0) {
      return i;
    }
  }

  return -1;
} 

