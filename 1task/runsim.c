#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <signal.h>


typedef union
{
    int val;
    struct semid_ds* buf;
    unsigned short* array;
} semctl_param;


int main(int argc, char ** argv){
    int maxTasks;
    if (argc != 2 || sscanf(argv[1], "%d", &maxTasks) != 1 || maxTasks <= 0){
        printf("Usage: program N\n"); 
        exit(1);
    }
    key_t key;
    if ((key = ftok(argv[0], 0)) < 0){
        printf("Failed to generate key\n");
        exit(1);
    }
    int sem_id; 
    if ((sem_id = semget(key, 1, 0666 | IPC_CREAT)) < 0){
        printf("Failed to create semaphore\n");
        exit(1);
    }
    semctl_param maxTasksParam = { maxTasks };
    if (semctl(sem_id, 0, SETVAL, maxTasksParam) < 0){
        printf("Failed to set initial semaphore value\n");
        exit(1);
    }
    signal(SIGCHLD, SIG_IGN);

    while (1){
        char line[1024];
        if (fgets(line, 1024, stdin) <= 0)
            break;
        
        char* arguments[128] = {};
        int nArguments = 0;
        int length = strlen(line);
        char* argumentBegin = line;

        for (int i = 0; i <= length; i++){
            if (line[i] == 0 || line[i] == ' ' || line[i] == '\n')
            {
                line[i] = 0;
                if (argumentBegin != line + i)
                    arguments[nArguments++] = argumentBegin;
                argumentBegin = line + i + 1;
            }
        }
        int childPID = fork();
        if (childPID == 0){
            struct sembuf operation; 
            operation.sem_num = 0;
            operation.sem_op = -1;  
            operation.sem_flg = SEM_UNDO | IPC_NOWAIT;

            if (semop(sem_id, &operation, 1) < 0){
                printf("Too many processes, only %d allowed.\n", maxTasks);
                exit(1);
            }
            if (execvp(arguments[0], arguments) < 0){
                printf("Failed to execute '%s': %s\n", arguments[0], strerror(errno));
                exit(1);
            }
        }
        else if (childPID < 0){
            printf("Failed to fork: %s\n", strerror(errno));
            exit(1);
        }
    }

    return 0;
}
