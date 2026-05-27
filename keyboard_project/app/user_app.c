#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int fd;

    char buffer[100];

    fd = open("/dev/kb_timeout", O_RDONLY);

    if(fd < 0)
    {
        perror("open");
        return 1;
    }

    while(1)
    {
        memset(buffer, 0, sizeof(buffer));

        lseek(fd, 0, SEEK_SET);

        read(fd, buffer, sizeof(buffer));

        printf("%s", buffer);

        sleep(1);
    }

    close(fd);

    return 0;
}
