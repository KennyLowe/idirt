#ifndef _MAIN_H
#define _MAIN_H

int	main (int argc, char **argv, char **ep);
char	*read_hosts (char search[20]);
void	process_data (char data[300], char *ip, char *host);
void	usesocketcom (void);

#endif
