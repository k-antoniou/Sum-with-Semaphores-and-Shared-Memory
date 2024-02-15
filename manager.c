#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>

static volatile sig_atomic_t counter = 0;

void handler(int sig){
    counter++;
}

void allocate2Darray(int rows,int cols,int***A){   
    int i;
    *A=(int**)malloc(rows*(sizeof(int*)));
    for(i=0;i<rows;i++){
        (*A)[i]=(int*)malloc(cols*(sizeof(int)));
    }
}

void free_array(int rows,int**A){
    int i;
    for(i=0;i<rows;i++){
        free(A[i]);
    }
    free(A);
}
void print2Darray(int rows,int cols,int **A){
    int i,j;
    for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
            printf("%d\t",A[i][j]);
        }
        printf("\n");
    }              
}
void fill_array(int rows,int cols, int **A){
    srand(time(0));
    int i,j;
    for (i=0;i<rows;i++){                   //filling array with randmon integers from 0 to 99
        for(j=0;j<cols;j++){
            A[i][j]=rand() %100;
        }
    }     
}
int array_sum(int size,int *sums){          
    int i,sum=0;
    for(i=0;i<size;i++){
        sum+=sums[i];
    }
    return sum;
}
void convert2Dto1D(int rows,int cols,int **A,int *B){
    int i,j,new_position;
     for(i=0;i<rows;i++){
        for(j=0;j<cols;j++){
            new_position=(i*cols)+j;
            B[new_position]=(A)[i][j];
        }
    }     
}
int main(int argc, char* argv[]){
    int i,rows,cols,**A,*B,*sharedMemory,key,shmid,pid,semid,*sums,sum;
    struct sigaction act = {{0}};
    rows=atoi(argv[1]);
    cols=atoi(argv[2]);
    allocate2Darray(rows,cols,&A);
    sums=(int*)malloc(rows*sizeof(int));
    B=(int*)malloc(rows*cols*sizeof(int));
    sharedMemory=(int*)malloc((5+rows+(rows*cols))*sizeof(int));
    key=ftok(".",20);
    shmid=shmget(key,1024,0666|IPC_CREAT);
    semid=semget(IPC_PRIVATE,1,S_IRWXU);   
    semctl(semid,0,SETVAL,0);
    act.sa_handler = handler;               //sending signal SIGUSR1 to handler
    sigaction(SIGUSR1, &act, NULL);
    pid=getpid();
    printf("\nShared memory key: %d\n\n",key);
    fill_array(rows,cols,A);
    convert2Dto1D(rows,cols,A,B);  
    printf("The 2D array is:\n");     
    print2Darray(rows,cols,A);
    sharedMemory=(int*)shmat(shmid,NULL,0);    //attaching to shared memory
    sharedMemory[0]=rows;
    sharedMemory[1]=cols;                       //sharedMemory[2] is used by the second program
    sharedMemory[3]=pid;
    sharedMemory[4]=semid;
    for(i=0;i<rows*cols;i++){
        sharedMemory[5+i]=B[i];
    }
    printf("\n\t\tWaiting....\nYou need to run %d task(s) to proceed\n", rows);
    while(counter<rows){                //waiting until all tasks are complete
        
    }
    for(i=0;i<rows;i++){
       sums[i]=sharedMemory[5+(rows*cols)+i];
    }
    sum=array_sum(rows,sums);
    printf("\nThe final sum equals: %d\n",sum);
    shmdt(sharedMemory);
    shmctl(shmid,IPC_RMID,NULL);
    semctl(semid,0,IPC_RMID);
    free_array(rows,A);
    free(B);
    free(sums);
    return(0);
}