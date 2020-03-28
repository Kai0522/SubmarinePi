//cross-process communication
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <python3.5/Python.h>
#include "mpu9250.h"

int sensorData=0;
int fd[2];

//Using Python function
int controlLawDesign(int x,int a, int b){
    PyObject *pModule, *pFunc, *pArgs, *pKargs, *pRes;
    int res;

    //import 'BIGNENE' module  &'complex_calc' function
    pModule=PyImport_Import(PyUnicode_FromString("submarineAlgorithm"));
    pFunc=PyObject_GetAttrString(pModule,"controlLawDesign");

    //turn arguments into PyObject
    pArgs=Py_BuildValue("(i)",x);
    pKargs=Py_BuildValue("{s:i,s:i}","a",a,"b",b);
    
    //call function
    pRes=PyObject_Call(pFunc,pArgs,pKargs);

    //check if calling is failed. If success, trans the PyObject into C type
    if(pRes == NULL) return -1;
    res=PyLong_AsLong(pRes);

    //process memory allocation
    Py_DECREF(pModule);
    Py_DECREF(pFunc);
    Py_DECREF(pArgs);
    Py_DECREF(pKargs);
    Py_DECREF(pRes);

    return res;
}

void gotSensorDataSig(int a){
    printf("get signal\n\r");
    read(fd[0],&sensorData,sizeof(sensorData));
}

int main(void){
    pipe(fd);
    const pid_t parrent_ID=getpid();
    pid_t child_ID=fork();

    switch(child_ID){

        //fork error occur 
        case -1:
            perror("failed to fork first times");
            exit(-1);
        //child process(sensor)
        
        //sensoring process
        case 0:
            close(fd[0]);
            initMPU9250();
            sleep(1);
            while(1){
                //virtual sensor
                sensorData=getData(ACCEL_XOUT_H);
                //send signal to parrent process
                kill(parrent_ID,SIGUSR1);
                //input sensor data into pipe.
                write(fd[1],&sensorData,sizeof(sensorData));
                sleep(1);
            };
            close(fd[1]);
            break;

        default:
            Py_Initialize();
            PySys_SetPath(Py_DecodeLocale(".",NULL));
            close(fd[1]);
            signal(SIGUSR1,gotSensorDataSig);
            while(1){
                printf("[parrent process]:sensorData=%d\n\r",controlLawDesign(sensorData,1,0));
                sleep(2);
            }
            close(fd[0]);
            Py_Finalize();
            break;
    }
    return 0; 
}
