//cross-process communication
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <python3.5/Python.h>

int pipein=0;
int sensorData=0;

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

void gettedSensorDataSig(int a){
    printf("get signal\n\r");
    pipein=1;
}

int main(void){
    int fd[2];
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
            while(1){
                //virtual sensor
                scanf("%d",&sensorData);
                printf("[child process]:getted sensor signal\n\r");
                //send signal to parrent process
                kill(parrent_ID,SIGUSR1);
                printf("[child process]:sended signal\n\r");
                //input sensor data into pipe.
                write(fd[1],&sensorData,sizeof(sensorData));
                printf("[child process]:inputted pipe\n\r");
            };
            close(fd[1]);
            break;

        default:
            Py_Initialize();
            PySys_SetPath(Py_DecodeLocale(".",NULL));
            close(fd[1]);
            signal(SIGUSR1,gettedSensorDataSig);
            while(1){
                if(pipein==1){
                    read(fd[0],&sensorData,sizeof(sensorData));
                    pipein=0;
                }
                //printf("[parrent process]:%d\n\r",sensorData);
                printf("[parrent process]%d\n\r",controlLawDesign(sensorData,1,0));
                sleep(1);
            }
            close(fd[0]);
            Py_Finalize();
            break;
    }
    return 0; 
}
