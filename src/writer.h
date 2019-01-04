#ifndef _WRITER_H
#define _WRITER_H

typedef struct _wr_line {
  struct _wr_line *next;
  char		  *s;
} WrLine;

typedef struct {
  void		*previous;
  char		*prevprompt;
  void		(*exit_handler)(void *w, void *arg, int arglen);
  void		(*old_inp_h)(char *str);
  char		*prompt;
  void		*arg;
  int		arglen;
  int		flags;
  int		max_lines;
  int		num_lines;
  WrLine	*first, *last;
} WrHead;

void start_writer(char *h,        /* Header line */
		  char *p,        /* Prompt string */
		  void *arg,      /* Argument data to handler. */
		  int  arglen,    /* Length of argument data */
		  void handler(void *x,void *arg,int arglen), /* handler */
		  int  flags,     /* Flags */
		  int max_lines); /* Max number of lines */

int wnum_lines(void *w); /* Get number of lines left in buffer */

int wnum_chars(void *w); /* Get number of chars (no \n) in buffer */


/* These functions are used by handler to read the text */
/* Get one character from writer, all memory is released
 * when EOF is returned. including the memory area for arg.
 * if arg is to be used after reading EOF, make sure to copy it
 * to a safe place before EOF is read by wgetc.
 */
int  wgetc(void *w);     /* get one character from writer */

/* Unget one character from writer, unget EOF is legal and effectively
 * empties the buffer
 */
int  wungetc(int c, void *w);
char *wgets(char *buf,int buflen,void *w); /* Get one line of characters */

/* terminate writer terminates the writer by first doing wungetc(EOF,x);
 * followed by reading that same EOF character and thereby
 * releasing all memory used by the writer.
 */
void terminate_writer(void *w);

/* Terminates all writers used by player plx. */
void terminate_all_writers(int plx);

void write_handler(char *line);

/* flags */
#define WR_CMD    0x0100    /* Commands are permitted */
#define WR_CMDCH    0x7f    /* Mask for special character, default '*' */

#endif
