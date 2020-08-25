#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <sys/mman.h>
#include <unistd.h>

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

    int buffer = 0x0;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    /* Returns a pointer to the 4GB point in /dev/mem - the start of my reserved memory. Only mapping 4096 bytes. */
    reserved_memory_1 = (int *) mmap(0, 4, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, RESERVED_MEMORY_OFFSET);

    printf("data: %x\n", buffer);

    //============================================
    buffer = to_fixed_32(5);
    reserved_memory_2 = reserved_memory_1 + 8; //Direccion 0x1ff00020

    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);

    //============================================
    buffer = to_fixed_32(22);
    reserved_memory_2 = reserved_memory_1 + 9; //Direccion 0x1ff00024
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);

    //============================================
    buffer = to_fixed_32(3.99);
    reserved_memory_2 = reserved_memory_1 + 10; //Direccion 0x1ff00028
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);


    //============================================
    buffer = to_fixed_32(0.1);
    reserved_memory_2 = reserved_memory_1 + 11; //Direccion 0x1ff0002c
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);


    //============================================
    buffer = to_fixed_32(0);
    reserved_memory_2 = reserved_memory_1 + 12; //Direccion 0x1ff00030
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);

    //============================================
    buffer = to_fixed_32(0);
    reserved_memory_2 = reserved_memory_1 + 14; //Direccion 0x1ff00038
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);


    //============================================
    buffer = to_fixed_32(100);
    reserved_memory_2 = reserved_memory_1 + 16; //Direccion 0x1ff00040
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);


    //============================================
    buffer = to_fixed_32(0.55);
    reserved_memory_2 = reserved_memory_1 + 18; //Direccion 0x1ff00048
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);


    //============================================
    buffer = to_fixed_32(-13);
    reserved_memory_2 = reserved_memory_1 + 20; //Direccion 0x1ff00050
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);


    //============================================
    buffer = to_fixed_32(1e-6);
    reserved_memory_2 = reserved_memory_1 + 22; //Direccion 0x1ff00058
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);

    //============================================
    buffer = 1;
    reserved_memory_2 = reserved_memory_1 + 24; //Direccion 0x1ff00060
    if (reserved_memory_2 == MAP_FAILED) {
        printf("Failed to creating mapping_2.\n");
        printf("ERRNO: %s\n", strerror(errno));
        return -1;
    }
    memcpy(reserved_memory_2, &buffer, 4);

    FILE *fp;
    int p1;
    int p2;
    int v;
    int i;
    //int aux = 1;

    fp=fopen("SIGNALS.CSV","w+");

    usleep(20);
    for (int t=0; t<4; t++){

        reserved_memory_2 = reserved_memory_1 + 4;
        memcpy(&p1, reserved_memory_2, 4);

        reserved_memory_2 = reserved_memory_1 + 5;
        memcpy(&p2, reserved_memory_2, 4);

        reserved_memory_2 = reserved_memory_1 + 6;
        memcpy(&v, reserved_memory_2, 4);

        reserved_memory_2 = reserved_memory_1 + 7;
        memcpy(&i, reserved_memory_2, 4);

        //memcpy(reserved_memory_1, &aux, 4);

        printf("Iteracion #: %d \n", t);
        printf("p1: %f \t", p1/pow(2,21));
        printf("p2: %f\t", p2/pow(2,21));
        printf("V: %f \t", v/pow(2,21));
        printf("I: %f \n", i/pow(2,21));

        fprintf(fp,"%f,%f,%f,%f\n",p1/pow(2,21),p2/pow(2,21),v/pow(2,21),i/pow(2,21));
    }
    fclose(fp);
    return 0;
}
