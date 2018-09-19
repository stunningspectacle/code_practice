/* ----------------------------------------------------------------------------
   Copyright (C) 2013 Intel Mobile Communications GmbH

        Sec Class: Intel Confidential (IC)

   All rights reserved.
   ----------------------------------------------------------------------------
   This document contains proprietary information belonging to IMC.
   Passing on and copying of this document, use
   and communication of its contents is not permitted without prior written
   authorisation.
  ---------------------------------------------------------------------------*/

#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 24

void memsetw(void *s, int c, unsigned int n)
{
  __asm__ __volatile__
  ("cld\n\t"
   "rep\n\t"
   "stosw": : "c"(n), "a"(c), "D"(s));
}

uint8_t make_color(enum vga_color fg, enum vga_color bg)
{
  return fg | bg << 4;
}

uint16_t make_vgaentry(char c, uint8_t color)
{
  uint16_t c16 = c;
  uint16_t color16 = color;
  return c16 | color16 << 8;
}

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

uint16_t local_buffer[VGA_WIDTH * VGA_HEIGHT];

void vga_cls(void)
{
  size_t x, y;
  for(y = 0; y < VGA_HEIGHT; y++)
  {
    for(x = 0; x < VGA_WIDTH; x++)
    {
      const size_t index = y * VGA_WIDTH + x;
      local_buffer[index] = make_vgaentry(' ', terminal_color);
    }
  }

  memcpy(terminal_buffer, local_buffer, VGA_WIDTH * VGA_HEIGHT * 2);
}

void vga_open(void)
{
  terminal_row    = 0;
  terminal_column = 0;
  terminal_color  = make_color(COLOR_WHITE, COLOR_BLACK);
  terminal_buffer = (uint16_t*) 0xB8000;
  vga_cls();
}

void terminal_setcolor(uint8_t color)
{
  terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
  const size_t index = y * VGA_WIDTH + x;
  local_buffer[index] = make_vgaentry(c, color);
}

void terminal_scroll()
{
  uint16_t i;
  uint16_t blank, temp;

  /* A blank is defined as a space... we need to give it
     *  backcolor too */
  blank = make_vgaentry(' ', terminal_color);

  /* Row 25 is the end, this means we need to scroll up */
  if(terminal_row >= VGA_HEIGHT)
  {
    /* Move the current text chunk that makes up the screen
        *  back in the buffer by a line */
    temp = terminal_row - VGA_HEIGHT + 1;
    memcpy(local_buffer, local_buffer + temp * VGA_WIDTH, (VGA_HEIGHT - temp) * VGA_WIDTH * 2);

    for(i = 0; i < VGA_WIDTH; i++)
    {
      local_buffer[(VGA_HEIGHT - temp) * VGA_WIDTH + i] = blank;
    }
    //memsetw(terminal_buffer + (VGA_HEIGHT - temp) * VGA_WIDTH , blank, VGA_WIDTH);
    terminal_row = VGA_HEIGHT - temp;
  }

  memcpy(terminal_buffer, local_buffer, VGA_WIDTH * VGA_HEIGHT * 2);
}

void terminal_putchar(char c)
{
  if(c == '\n')
  {
    terminal_column = 0;
    terminal_row++;
  }
  else if(c >= ' ')
  {
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    terminal_column++;
    if(terminal_column == VGA_WIDTH)
    {
      terminal_column = 0;
      terminal_row++;
    }
  }

  terminal_scroll(); //scroll the screen
}

int delay_total;
void vga_print_text(char* data)
{
  size_t i, j;
  size_t datalen = strlen(data);
  for(i = 0; i < datalen; i++)
  {
    /*
        for(j = 0; j < 10000000; j++)
        {
          delay_total += j;
        }
    */
    terminal_putchar(data[i]);
  }
}

void vga_set_color(enum vga_color fg, enum vga_color bg)
{
  terminal_color = make_color(fg, bg);
}

void vga_update(void)
{

}

