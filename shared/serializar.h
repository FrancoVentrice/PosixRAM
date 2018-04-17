#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

int serializar(void* buffer, const char* format, ...);
int deserializar(void* buffer, const char* format, ...);