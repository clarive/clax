#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "scandir.h"

/* From MPlayer */

int
alphasort(const struct dirent **a, const struct dirent **b)
{
    return strcoll((*a)->d_name, (*b)->d_name);
}

int
scandir(const char *dirname,
    struct dirent ***ret_namelist,
    int (*select)(const struct dirent *),
    int (*compar)(const struct dirent **, const struct dirent **))
{
    int i, len;
    int used, allocated;
    DIR *dir;
    struct dirent *ent, *ent2;
    struct dirent **namelist = NULL;

    if ((dir = opendir(dirname)) == NULL)
    return -1;

    used = 0;
    allocated = 2;
    namelist = malloc(allocated * sizeof(struct dirent *));
    if (!namelist)
    goto error;

    while ((ent = readdir(dir)) != NULL) {

    if (select != NULL && !select(ent))
        continue;

    /* duplicate struct direct for this entry */
    len = offsetof(struct dirent, d_name) + strlen(ent->d_name) + 1;
    if ((ent2 = malloc(len)) == NULL)
        return -1;

    if (used >= allocated) {
        allocated *= 2;
        namelist = realloc(namelist, allocated * sizeof(struct dirent *));
        if (!namelist)
        goto error;
    }
    memcpy(ent2, ent, len);
    namelist[used++] = ent2;
    }
    closedir(dir);

    if (compar)
    qsort(namelist, used, sizeof(struct dirent *),
          (int (*)(const void *, const void *)) compar);

    *ret_namelist = namelist;
    return used;


error:
    if (namelist) {
    for (i = 0; i < used; i++)
        free(namelist[i]);
    free(namelist);
    }
    return -1;
}
