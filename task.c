#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>


int main(int argc, char* argv[]){
    int i,shmid,rows,cols,task_id,*B,pid,semid,*sharedMemory,*sums,key;
    struct sembuf op;
    key=atoi(argv[1]); 
    shmid=shmget(key,1024,0666|IPC_CREAT);
    sharedMemory=(int*)shmat(shmid,NULL,0);         //attaching to Shared Memory
    rows=sharedMemory[0];
    cols=sharedMemory[1];
    B=(int*)malloc(rows*cols*(sizeof(int)));
    sums=(int*)malloc(rows*sizeof(int));
    task_id=sharedMemory[2];
    sharedMemory[2]++;          //is used by the next task
    pid=sharedMemory[3];
    semid=sharedMemory[4];
    for(i=0;i<rows*cols;i++){
       B[i]=sharedMemory[5+i];
    }
    
    if (task_id==rows-1){       
        op.sem_num=0;
        op.sem_op=rows;
        op.sem_flg=0;
        semop(semid,&op,1);
    }else{
    printf("\t\tWaiting....\nYou need to run %d more task(s) to proceed\n",rows-(task_id+1));
    op.sem_num=0;
    op.sem_op=-1;
    op.sem_flg=0;
    semop(semid,&op,1);
    }
    
    for(i=task_id*cols;i<(cols*task_id)+cols;i++){          //calculates the sum of the row
        sums[task_id]=sums[task_id]+B[i];
    }
    sharedMemory[5+(rows*cols)+task_id]=sums[task_id];
    sleep(task_id);
    kill(pid,SIGUSR1);      //sends signal to manager
    printf("\nSum of row %d: %d\n",task_id+1,sums[task_id]);
    shmdt(sharedMemory);
}