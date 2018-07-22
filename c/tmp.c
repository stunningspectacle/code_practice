#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#define MYEXEC(n) \
	do { \
		__asm__ __volatile__("int $"#n"\n\t");\
	} while (0)

const char *all_path = "/sys/bus/i2c/devices/i2c-0/name   ; \
                        /sys/bus/i2c/devices/i2c-1/name";
static char *trim_space(char *str)
{
    char *end;

    while (isspace(*str))
        str++;
    if (*str == 0)
        return NULL;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        *end-- = 0;

    printf("xxxx%sxxxx\n", str);
    return str;
}

int myint(int n) {
	__asm__ __volatile__( "int $0\n\t" );
}

int main(void)
{
    char path[256] = { 0 };
    char *str, *str_left;
    int count = 0;
    strncpy(path, all_path, sizeof(path));
    str_left = path;
    while ((str = strsep(&str_left, ";")) != NULL) {
        printf("count = %d, str=%s\n", count++, str);
        if ((str = trim_space(str)) == 0)
            printf("xxxxxxxxxxx\n");
        else if (access(str, F_OK) == 0)
            printf("OK! yyyyy%syyyyy\n", str);
        else
            printf("Failed! yyyyy%syyyy: %s\n", str, strerror(errno));
    }
	MYEXEC(1);
}
