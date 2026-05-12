#include <stdio.h>

#include "app_info.h"

int main(void)
{
    printf("%s server %s\n", lanchess_name(), lanchess_version());
    printf("Server placeholder started. Chess, networking, and concurrency logic are not implemented yet.\n");
    return 0;
}
