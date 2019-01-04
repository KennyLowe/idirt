#ifndef _UTILS_H_
#define _UTILS_H_

/************************************************************************
 * Structures								*
 ************************************************************************/
/* int_table Interface */
typedef struct TABLE_ENTRY {
  long int key;
  long int value;
  struct TABLE_ENTRY *next;
} table_entry;

typedef struct {
  int  len;
  table_entry **table;
} int_table;

/* For the Int-Set Package */
typedef struct {
  int  current;	/* Current element */
  int  len;	/* # of elements in the array */
  int  maxlen;	/* Length of array */
  int *list;	/* Pointer to the array */
} int_set;

/************************************************************************
 * Macros/Defines							*
 ************************************************************************/
#define int_number(I, P) ((P)->list[I])

#define first_int(P) ((P)->len == 0 ? SET_END : ((P)->current = 1, *(P)->list))
#define next_int(P)  ((P)->current < (P)->len ? \
		      (P)->list[(P)->current++] : SET_END)

#define set_size(P) ((P)->len)
#define is_empty(P) (set_size(P) == 0)

#define NOT_IN_TABLE -65536L
#define SET_END -65536
#define TABLE_END ((char *)(-1))

#define MAGIC_ZHEADER	'\037'
#define MAGIC_GZIP	'\213'
#define MAGIC_COMPRESS	'\235'
#define	IS_GZIP		1
#define	IS_COMPRESS	2

/************************************************************************
 * Prototypes								*
 ************************************************************************/
int	cmp_player(const void *a, const void *b);
int	cmp_alpha(const void *a, const void *b);

void	init_rand(void);
int	randperc(void);
Boolean	match (char *p, char *q);
Boolean	infile(char *file, char *line);

int	strtlookup(char *elem, char **table, int *begin, int *end);
int	glookup(char *elem,int n,char **table,
		int (*strcmpfun)(const char *s1, const char *s2, size_t n));
int	tlookup(char *elem,char **table);
int	xlookup(char *elem,char **table);
char	*lowercase(char *str);
char	*uppercase(char *str);

void	xfree(void *ptr);
void	*xmalloc(int nelem, int elem_size);
void	*resize_array(void *start, int elem_size, int oldlen, int newlen);

int	addordel(char file_name[80], char check[80]);
int	addname(char file_name[80], char name[80]);
int	delname(char file_name[80], char name[80]);

int	fnumlines(char file[200]);
void	fileseek(FILE *file, int lines);

Boolean	tstbits(int w, int m);
Boolean	tst_bit(LongInt *f,int b);
void	set_bit(LongInt *f,int b);
void	clr_bit(LongInt *f,int b);
Boolean	dtst_bit(DLongInt *f, int b);
void	dset_bit(DLongInt *f,int b);
void	dclr_bit(DLongInt *f,int b);

void	insertch(char *s, char ch, int i);
char	*my_crypt(char *buf,char *pw,int len);
char	*mk_string(char *b,char *str,int k,int stopch);
char	*x_strcpy(char *d, char *s);

/* int_set interface */
void	init_intset(int_set *p, int len);
void	free_intset(int_set *p);
Boolean	add_int(int n, int_set *p);
Boolean	remove_int(int n, int_set *p);
int	find_int(int n, int_set *p);
int	find_int_number(int n, int_set *p);
int	foreach_int(int_set *p, int (*func)(int));
void	remove_current(int_set *p);
int	get_set_mem_usage(int_set *p);

void	init_inttable(int_table *p, int size);
void	free_inttable(int_table *p);

Boolean	insert_entry(long int key, long int value, int_table *p);
Boolean	remove_entry(long int key, int_table *p);
long	lookup_entry(long int key, int_table *p);
long	change_entry(long int key, long int new_value, int_table *p);
int	get_table_mem_usage(int_table *p);

void	write_date_stderr (void);
int	check_file_magic (char *filename);
FILE	*zopen (char *filename);
void	zclose (FILE *fp);

#ifdef SYS_EQBUG
int strcasecmp(char *s1, char *s2);
int strncasecmp(const char *s1, const char *s2, size_t n);
#endif

#endif
