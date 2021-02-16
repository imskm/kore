#ifndef _DOTENV_H
#define _DOTENV_H

#include <stdio.h>

#define DOTENV_MAX_SIZE 	100
#define DOTENV_FILENAME 	".env"
#define DOTENV_LIBNAME 		"dotenv"
#define DOTENV_MAXLINE 		256

void dotenv_init(void);
void dotenv_teardown(void);


#endif
