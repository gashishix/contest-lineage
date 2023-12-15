#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <signal.h>

// table with MESSAGES

int TABLE_LIMIT = 0;

int new_msgq(const char* p, int id){
    // creates messages queue
    key_t key;
    int qid; // queue id

    if ((key = ftok(p, id)) < 0){
        printf("ftok error\n");
        exit(1);
    }
    if ((qid = msgget(key, 0666 | IPC_CREAT)) < 0){
        printf("msgget error\n");
        exit(1);
    }

    return qid;
}

void rm_msgq(int id){
    // removes messages queue
    msgctl(id, IPC_RMID, (struct msqid_ds*)0);
}

struct TABLEWARE{
    // struct for each thing in queue
    int type; //fork, spoon, etc.
    int wash_sec;
    int dry_sec;
};

struct MESSAGE{ // check p.411 for reference
    long mtype;
    int type;
    union
    {
        int wait_time;
        int extra[10];
    };
};

struct MESSAGE msg;

int main(int argc, char** argv){
    int pid; // process id
    int q0, q1; // queues
    int i;

    int n, type, wash_sec, dry_sec;
    //char type;
    int count, index;
    memset(&msg, 0, sizeof(msg));

    if (argc != 4){
        printf("Arguments fault, input must be 1 int and 2 file\n");
        exit(1);
    }

    sscanf(argv[1], "%d", &TABLE_LIMIT);
    
    pid = fork();
    signal(SIGCHLD, SIG_IGN);

    if (pid == 0){ // child = dryer
        q0 = new_msgq(argv[0], 0);
        q1 = new_msgq(argv[0], 1);

        printf("-------> DRYER <-------\n\n");

        for (i = 0; i < TABLE_LIMIT; i++){ // creating empty table
            memset(&msg, 0, sizeof(msg));
            msg.mtype = 10; 
            if (msgsnd(q1, &msg, sizeof(msg) - sizeof(long), 0) < 0){
                printf("msgsnd error\n");
                exit(1);
            }
        }

        while (1){
            
            memset(&msg, 0, sizeof(msg));
            if (msgrcv(q0, &msg, sizeof(msg) - sizeof(long), 0, 0) < 0){
                printf("msgrcv error97\n");
                exit(1);
            }

            if (msg.mtype == 20) // exit
                break;
            
            printf("%d BEGIN DRYING\n", msg.type);
            sleep(msg.wait_time);
            printf("%d END   DRYING\n", msg.type);

            msg.mtype = 10; //empty table

            if (msgsnd(q1, &msg, sizeof(msg) - sizeof(long), 0) < 0){
                printf("msgsnd error\n");
                exit(1);
            }
        }
    } else if (pid > 0){ // parent = washer
        struct TABLEWARE thing[50];
        n = 0;

        FILE* list = fopen(argv[2], "r");
        if (!list){
            printf("Type file error121\n");
            exit(1);
        }

        while (fscanf(list, "%d: %d %d", &type,
                    &wash_sec, &dry_sec) == 3){
            thing[n].type = type;
            thing[n].wash_sec  = wash_sec;
            thing[n].dry_sec = dry_sec;
            printf("TABLEWARE %d wash %d sec, dry %d sec\n", type, wash_sec, dry_sec);
            n++;
        }
        fclose(list);

        q0 = new_msgq(argv[0], 0);
        q1 = new_msgq(argv[0], 1);

        list = fopen(argv[3], "r");
        if (!list){
            printf("Input file error142\n");
            exit(1);
        }

        printf("-------> WASHER <-------\n\n");

        while (fscanf(list, "%d: %d", &type, &count) == 2){
            index = -1;
            for (i = 0; i < n; i++)
                if (thing[i].type == type){
                    index = i;
                    break;
                }
            if (index == -1){
                printf("thing type error\n");
                exit(1);
            }

            printf("\nthing %d: amount: %d, index: %d\n", type, count, index);

            while (count){

                if (msgrcv(q1, &msg, sizeof(msg) - sizeof(long), 0, 0) < 0){
                    printf("msgrcv error (parent from child)\n");
                    exit(1);
                }

                printf("%d BEGIN WASHING\n", thing[index].type);
                sleep(thing[index].wash_sec);
                printf("%d END   WASHING\n", thing[index].type);

                memset(&msg, 0, sizeof(msg));
                msg.mtype = 30; // dry thing
                msg.type = thing[index].type;
                msg.wait_time = thing[index].dry_sec;

                if (msgsnd(q0, &msg, sizeof(msg) - sizeof(long), 0) < 0){
                    printf("msgsnd error (parent -> child)\n");
                    exit(1);
                }

                count--;
            }
        }

        fclose(list);

        memset(&msg, 0, sizeof(msg));
        msg.mtype = 20; // exit

        if (msgsnd(q0, &msg, sizeof(msg) - sizeof(long), 0) < 0){
            printf("msgsnd error (exit, parent -> child)\n");
            exit(1);
        }
        wait(0);
    } else {
        printf("fork error\n");
        exit(1);
    }

    rm_msgq(q0);
    rm_msgq(q1);
    return(0);
}
