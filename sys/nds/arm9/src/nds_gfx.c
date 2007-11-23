#include <nds.h>
#include "font-bdf.h"
#include "ppm-lite.h"

void nds_draw_hline(int x, int y, int width, u8 colour, u16 *dest)
{
  int i;

  dest += (y * 128) + (x / 2);

  if (x & 0x01) {
    *dest = (*dest & 0x00FF) | (colour << 8);
    dest++;
  }

  for (i = 0; i < (width - ((x & 0x01) ? 1 : 0)) / 2; i++, dest++) {
    *dest = (colour << 8) | colour;
  }

  if ((x + width) & 0x01) {
    *dest = (*dest & 0xFF00) | colour;
  }
}

void nds_draw_vline(int x, int y, int height, u8 colour, u16 *dest)
{
  int i;

  dest += (y * 128) + (x / 2);

  for (i = 0; i < height; i++, dest += 128) {
    if (x & 0x01) {
      *dest = (*dest & 0x00FF) | (colour << 8);
    } else {
      *dest = (*dest & 0xFF00) | colour;
    }
  }
}

void nds_draw_rect(int x, int y, int width, int height, u8 colour, u16 *dest)
{
  int i;
  u16 tmp = (colour << 8) | colour;

  for (i = 0; i < height; i++, dest += 128) {
    nds_draw_hline(x, y, width, tmp, dest);
  }
}

void nds_draw_rect_outline(int x, int y, int width, int height, u8 fill_colour, u8 line_colour, u16 *dest)
{
  nds_draw_rect(x, y, width, height, fill_colour, dest);

  nds_draw_hline(x, y, width, line_colour, dest);
  nds_draw_hline(x, y + height, width, line_colour, dest);

  nds_draw_vline(x, y, height, line_colour, dest);
  nds_draw_vline(x + width, y, height, line_colour, dest);
}

void nds_draw_text(struct font *fnt, 
                   char *str,
                   int x, int y,
                   u16 black, u16 white,
                   u16 *dest)
{
  int w, h;
  struct ppm *img;

  text_dims(fnt, str, &w, &h);
  
  img = alloc_ppm(w, h);

  draw_string(fnt, str, img, 0, 0, 1, 255, 0, 255);

  draw_ppm_bw(img, dest, x, y, 256, black, white);

  free_ppm(img);
}

void nds_fill(u16 *dest, u8 colour)
{
  int i;
  u16 tmp = (colour << 8) | colour;

  for (i = 0; i < 256 * 192 / 2; i++) {
    dest[i] = tmp;
  }
}
