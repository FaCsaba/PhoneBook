#pragma once

#define uint unsigned int

#define RQ_ERROR_FILE_READING          1
#define RQ_ERROR_ENUM_VIOLATION        2
#define RQ_ERROR_VIRTUAL_MODE          3
#define RQ_ERROR_NOT_IMPLEMENTED_YET   4
#define RQ_ERROR_NOT_ENOUGH_MEMORY     5

#include <stdio.h>
/* Panic macro because I am used to how rust handels it's errors */
#define panic( msg, errco ) { printf("Paniced at %s:%i with:\n%s\nwith error code: %i\n", __FILE__, __LINE__, msg, errco); exit(errco); }

