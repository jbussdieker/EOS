////////////////////////////////////////////////////////////////////////////////
// $Id: dirent.h,v 1.1 2010/12/12 23:57:31 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _DIRENT_H
#define _DIRENT_H

struct dirent
{
	char name[128];
};

typedef void DIR;

int            closedir(DIR *);
DIR           *opendir(const char *);
struct dirent *readdir(DIR *);
//int            readdir_r(DIR *, struct dirent *, struct dirent **);
//void           rewinddir(DIR *);
//void           seekdir(DIR *, long int);
//long int       telldir(DIR *);

#endif