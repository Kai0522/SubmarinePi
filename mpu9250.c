#include "mpu9250.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>

int fd;

void initMPU9250(void){
    wiringPiSetup();
    fd=wiringPiI2CSetup(SlaveAddress);
    if(fd>=0){
        printf("fd=%d\n\r",fd);
        wiringPiI2CWriteReg8(fd,PWR_MGMT_1,0x00);
        wiringPiI2CWriteReg8(fd,SMPLRT_DIV,0x07);
        wiringPiI2CWriteReg8(fd,CONFIG,0x06);
        wiringPiI2CWriteReg8(fd,GYRO_CONFIG,0x18);
        wiringPiI2CWriteReg8(fd,ACCEL_CONFIG,0x01);
    }
    else{
        printf("failed to initialize MPU9250");
    }
}
int getData(int reg_address){
    return (wiringPiI2CReadReg8(fd,reg_address)<<8) + wiringPiI2CReadReg8(fd,reg_address+1);
}
