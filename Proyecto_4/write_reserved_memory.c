#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>

#define RESERVED_MEMORY_OFFSET  0x1ff00000  /* Offset */

int to_fixed_32(float a){
    a=a*pow(2,21);
    int b = (int)a;
    return b;
}


int main() {
    int fd;
    int *reserved_memory_1;
    int *reserved_memory_2;

    int buffer_1 = 0x1;
    int buffer_2 = 0x0;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    /* Returns a pointer to the 4GB point in /dev/mem - the start of my reserved memory. Only mapping 4096 bytes. */
    reserved_memory_1 = (int *) mmap(0, 4, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, RESERVED_MEMORY_OFFSET);

    printf("data: %x\n", buffer_1);
    printf("data: %x\n", buffer_2);

    //============================================
    buffer_2 = 1;
    reserved_memory_2 = reserved_memory_1 + 8; //Direccion 0x1ff00020

    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);

    //============================================
    buffer_2 = 2;
    reserved_memory_2 = reserved_memory_1 + 9; //Direccion 0x1ff00024
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);

    //============================================
    buffer_2 = 3;
    reserved_memory_2 = reserved_memory_1 + 10; //Direccion 0x1ff00028
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);


    //============================================
    buffer_2 = 4;
    reserved_memory_2 = reserved_memory_1 + 11; //Direccion 0x1ff0002c
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);


    //============================================
    buffer_2 = 5;
    reserved_memory_2 = reserved_memory_1 + 12; //Direccion 0x1ff00030
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);

    //============================================
    buffer_2 = 6;
    reserved_memory_2 = reserved_memory_1 + 14; //Direccion 0x1ff00038
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);


    //============================================
    buffer_2 = 7;
    reserved_memory_2 = reserved_memory_1 + 16; //Direccion 0x1ff00040
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);


    //============================================
    buffer_2 = 8;
    reserved_memory_2 = reserved_memory_1 + 18; //Direccion 0x1ff00048
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);


    //============================================
    buffer_2 = 9;
    reserved_memory_2 = reserved_memory_1 + 20; //Direccion 0x1ff00050
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);


    //============================================
    buffer_2 = 10;
    reserved_memory_2 = reserved_memory_1 + 22; //Direccion 0x1ff00058
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer_2, 4);

    //============================================
    buffer_2 = 11;
    reserved_memory_2 = reserved_memory_1 + 24; //Direccion 0x1ff00060
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

    memcpy(&buffer_1, reserved_memory_1 + 22, 4);
    printf("\n\n buffer_1: %d \n", buffer_1);
    return 0;
}