#include "rq_common.h"
#include <string.h>
#include <stdlib.h>

/* Stolen from: https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C */
uint distance(const char *source, const char *target) {
    uint x, y, s1len, s2len;
    s1len = strlen(source);
    s2len = strlen(target);
    uint matrix[s2len+1][s1len+1];
    matrix[0][0] = 0;
    for (x = 1; x <= s2len; x++)
        matrix[x][0] = matrix[x-1][0] + 1;
    for (y = 1; y <= s1len; y++)
        matrix[0][y] = matrix[0][y-1] + 1;
    for (x = 1; x <= s2len; x++)
        for (y = 1; y <= s1len; y++)
            matrix[x][y] = min(matrix[x-1][y] + 1, min(matrix[x][y-1] + 1, matrix[x-1][y-1] + (source[y-1] == target[x-1] ? 0 : 1)));

    return(matrix[s2len][s1len]);
}