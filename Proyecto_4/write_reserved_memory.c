
    #include <errno.h>
    #include <fcntl.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/mman.h>

    #define RESERVED_MEMORY_OFFSET  0x1ff00000     /* Offset */

    int main() {
            int fd;
            int *reserved_memory;
            int buffer;
            buffer = 16;

            fd = open("/dev/mem", O_RDWR | O_SYNC);
            /* Returns a pointer to the 4GB point in /dev/mem - the start of my reserved memory. Only mapping 4096 bytes. */
            reserved_memory  = (int *) mmap(0, 1, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, RESERVED_MEMORY_OFFSET);

            printf("data: %x\n", buffer);
            if (reserved_memory == MAP_FAILED) {
                    printf("Failed to creating mapping.\n");
                    printf("ERRNO: %s\n", strerror(errno));
                    return -1;
            }
	    memcpy(reserved_memory, &buffer, 4);

            //sprintf(reserved_memory, "%d", buffer);
            //printf("\n Print reseved\n\n");
            //printf("data  reseved: %x \n", aux);
            return 0;
    }
