#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define RESERVED_MEMORY_OFFSET  0x1ff00000    /* Offset */

int main() {
    int fd;
    int *reserved_memory_1;
    int *reserved_memory_2;

    int buffer_1 = 0xFFFF;
    int buffer_2 = 0xBB23;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    /* Returns a pointer to the 4GB point in /dev/mem - the start of my reserved memory. Only mapping 4096 bytes. */
    reserved_memory_1 = (int *) mmap(0, 4, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, RESERVED_MEMORY_OFFSET);

    reserved_memory_2 = reserved_memory_1 + 1; //Direccion 0x1ff00004

    printf("data: %x\n", buffer_1);
    printf("data: %x\n", buffer_2);

    //============================================
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);

    //============================================
    if (reserved_memory_1 == MAP_FAILED) {
        printf("Failed to creating mapping_1.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_1, &buffer_1, 4);
    return 0;
}

