#include <stdio.h>

#include "app_info.h"

int main(void)
{
    printf("%s client %s\n", lanchess_name(), lanchess_version());
    printf("Client placeholder started. Chess and networking logic are not implemented yet.\n");
    return 0;
}
