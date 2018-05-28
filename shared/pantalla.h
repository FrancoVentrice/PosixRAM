#ifndef _PANTALLA_H
#define _PANTALLA_H

#include <stdio.h>
#include <string.h>

/* se puede usar el escape \033 o \x1b */
#define RESET      "\x1b[0m"
/* colores de texto */
#define NEGRO_T    "\x1b[30m"
#define ROJO_T     "\x1b[31m"
#define VERDE_T    "\x1b[32m"
#define AMARILLO_T "\x1b[33m"
#define AZUL_T     "\x1b[34m"
#define MAGENTA_T  "\x1b[35m"
#define CYAN_T     "\x1b[36m"
#define BLANCO_T   "\x1b[37m"
/* colores de fondo */
/* modo mixto, se puede usar color de fondo en la misma sentencia:  \x1b[47;30m  (fondo;texto)*/
#define NEGRO_F    "\x1b[40m"
#define ROJO_F     "\x1b[41m"
#define VERDE_F    "\x1b[42m"
#define AMARILLO_F "\x1b[43m"
#define AZUL_F     "\x1b[44m"
#define MAGENTA_F  "\x1b[45m"
#define CYAN_F     "\x1b[46m"
#define BLANCO_F   "\x1b[47m"
/* efectos */
/* uso:  \033[#;#m 	where first # is the effect and second # is the color number */
#define BOLD      "\033[1m"    /* Bold */
#define DARK      "\033[2m"    /* Darker color */
#define UNDERLINE "\033[4m"    /* Underline */
#define BACKGND   "\033[7m"    /* Background */
#define STRIKE    "\033[9m"    /* Strike */
/* cursor */
/* To move the cursor you have to print "\x1b[r;cH", where r and c are respectively
  the row and column where you want your cursor to move */
#define CURSOR_UP    "\033[A"  // moves cursor one line above (carfull: it does not erase the previously written line)
#define CURSOR_DOWN  "\033[B"  // moves cursor one line under
#define CURSOR_RIGHT "\033[C"  // moves cursor one spacing to the right
#define CURSOR_LEFT  "\033[D"  // moves cursor one spacing to the left
#define RESET_POS    "\e[1;1H" // move cursor to line 1 column 1
/* pantalla */
#define CLEAN_LINES  "\033[2K" // erases everything written on line before this
#define CLEAR_SCR    "\e[2J"   // clear all the screen

void centrarTexto(char *);
void limpiarPantalla();
void mostrarTextoXY(int, int, char *);

#endif
