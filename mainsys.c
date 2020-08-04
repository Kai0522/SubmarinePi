//cross-process communication
#include <signal.h>
#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <python3.7/Python.h>

int Actuator;
int Sensor;
char sensorData[1024];
int pipe_fd[2];

//Using Python function
int controlLawDesign(char* x, int a, int b){
    PyObject *pModule, *pFunc, *pArgs, *pKargs, *pRes;
    int res;

    //import  module & function
    //PySys_SetPath(Py_DecodeLocale(".",NULL));

    pModule=PyImport_Import(PyUnicode_FromString("submarineAlgorithm"));
    pFunc=PyObject_GetAttrString(pModule,"controlLawDesign");

    //turn arguments into PyObject
    pArgs=Py_BuildValue("(s)", x);
    pKargs=Py_BuildValue("{s:i,s:i}","a",a,"b",b);
    //call function
    pRes=PyObject_Call(pFunc,pArgs,pKargs);

    //check if calling is failed. If success, trans the PyObject into C type
:x


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
    read(pipe_fd[0],&sensorData,sizeof(sensorData));
}

int main(void){
    //Open the ACM file
    int ACM_fd0,ACM_fd1;
    ACM_fd0=open("/dev/ttyACM0",O_RDWR);
    ACM_fd1=open("/dev/ttyACM1",O_RDWR);
    if(ACM_fd0==-1) perror("Can't create ACM0");
    if(ACM_fd1==-1) perror("Can't create ACM1"); 

    //Config the UART
    struct termios Opt0;
    tcgetattr(ACM_fd0,&Opt0);
    cfsetispeed(&Opt0,B9600);
    tcsetattr(ACM_fd0,TCSANOW,&Opt0);

    struct termios Opt1;
    tcgetattr(ACM_fd1,&Opt1);
    cfsetispeed(&Opt1,B9600);
    tcsetattr(ACM_fd1,TCSANOW,&Opt1);
    sleep(2);

    //If ACM_fd0 is Actuator or Sensor.
    while(1){
        char buff[1024];
        memset(&buff,'\0',sizeof(buff));
        int  Len=sizeof(buff);
        write(ACM_fd0,"name",4);
        tcflush(ACM_fd0,TCIOFLUSH);
        read(ACM_fd0,buff,Len);
        if (strcmp(buff,"My name is Bitchduino.\n")==0){
            printf("[pi]:Actuator Connected\n");
            Actuator=ACM_fd0;
            break;
        }
        if (strcmp(buff,"My name is Anarduino.\n")==0){
            printf("[pi]:Sensor Connected\n");
            Sensor=ACM_fd0;
            break;
        }
        printf("%s",buff);
        usleep(100000);
   }
    //If ACM_fd1 is Actuator or Sensor.
    while(1){
        char buff[1024];
        memset(&buff,'\0',sizeof(buff));
        int  Len=sizeof(buff);
        write(ACM_fd1,"name",4);
        tcflush(ACM_fd1,TCIOFLUSH);
        read(ACM_fd1,buff,Len);
        if (strcmp(buff,"My name is Bitchduino.\n")==0){
            printf("[pi]:Actuator Connected\n");
            Actuator=ACM_fd1;
            break;
        }
        if (strcmp(buff,"My name is Anarduino.\n")==0){
            printf("[pi]:Sensor Connected\n");
            Sensor=ACM_fd1;
            break;
        }
        printf("%s",buff);
        usleep(100000);
   }

    //Creat unnamed pipe
    pipe(pipe_fd);
    //get parrent process ID
    const pid_t parrent_ID=getpid();
    //Build Children process
    pid_t child_ID=fork();

    switch(child_ID){

        //fork error occur 
        case -1:
            perror("failed to fork first times");
            exit(-1);
        //child process(sensor)
        
        //sensoring process
        case 0:{
            char buff[110];
            int  Len=sizeof(buff);
            close(pipe_fd[0]);
            sleep(1);
            while(1){
            //virtual sensor
                memset(&buff,'\0',sizeof(buff));
                write(Sensor,"all",3);
                tcflush(Sensor,TCIOFLUSH);
                read(Sensor,buff,Len);
                tcflush(Sensor,TCIOFLUSH);
                fflush(stdout);
                printf("[child process] : %s",buff);
                //send signal to parrent process
                kill(parrent_ID,SIGUSR1);
                //input sensor data into pipe.
                write(pipe_fd[1],&buff,sizeof(buff));
                sleep(1);
            };
            close(pipe_fd[1]);
            break;
        }

        default:{
            Py_Initialize();
            PyObject *sysPath = PySys_GetObject((char*)"path");
            PyList_Append(sysPath,PyUnicode_FromString("."));

            close(pipe_fd[1]);
            signal(SIGUSR1,gotSensorDataSig);
            while(1){
                printf("[parrent process]:sensorData=%d\n\r",controlLawDesign(sensorData,1,0));
                fflush(stdout);
                //printf("[parrent process] : %s",sensorData);
                write(Actuator,"1",1);
                tcflush(Actuator,TCIOFLUSH);
                usleep(100000);
                write(Actuator,"0",1);
                tcflush(Actuator,TCIOFLUSH);
                sleep(2);
            }
            close(pipe_fd[0]);
            Py_Finalize();
            break;
        }
    }
    return 0; 
}
