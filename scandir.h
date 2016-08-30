#ifndef SCANDIR_H
#define SCANDIR_H

#if defined(_WIN32) || defined(MVS)

#include <dirent.h>

int
alphasort(const struct dirent **a, const struct dirent **b);

int
scandir(const char *dirname,
	struct dirent ***ret_namelist,
	int (*select)(const struct dirent *),
	int (*compar)(const struct dirent **, const struct dirent **));

#endif

#endif
