#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

void cleanUp(int n);
void setValue(int semset_id, int semnum, int value);
void get_lock(int semset_id);
void release_lock(int semset_id);

union semun{ 
	int val; 
	struct semid_ds* buf; 
	ushort* array; 
};

int seg_id;
int semset_id;
char* memptr;

int main(int argc, char** argv){

	char array[256];
	int pid;

	if( (seg_id = shmget(99,256,IPC_CREAT|0777)) == -1 ){
		perror("shmget:");
		exit(1);
	}

	memptr = shmat(seg_id,NULL,0);

	if( memptr == (void*) -1 ){
		perror("shmat:");
		exit(1);
	}

	semset_id = semget(9900,2,(0666|IPC_CREAT|IPC_EXCL));

	if( semset_id == -1 ){
		perror("semset");
		exit(1);
	}

	setValue(semset_id,0,0);
	setValue(semset_id,1,0);

	printf("SERVER STARTTED UP FINE\n");

	pid = getpid();

	while(1){

		get_lock(semset_id);

		strcpy(array,memptr);

		memptr[0] = '\0';

		release_lock(semset_id);

		if( strcmp(array,"QUIT") == 0 ){
			cleanUp(0);
		}

		if( strcmp(array,"PID") == 0 ){
			printf("Server pid: %d\n",pid);
		}

		if( strcmp(array,"HI") == 0 ){
			printf("Grettings!\n");
		}
	}

	return 0;
}

void cleanUp(int n){
	shmdt(memptr);
	shmctl(seg_id,IPC_RMID,NULL);
	semctl(semset_id,0,IPC_RMID,NULL);
	exit(n);
}

void setValue(int semset_id, int semun, int value){

	union semun initial_value;

	initial_value.val = value;

	if( semctl(semset_id,semun,SETVAL,initial_value) == -1 ){
		perror("semctl:");
		exit(1);
	}
}

void get_lock(int semset_id){

	struct sembuf actions[2];

	actions[0].sem_num = 1;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = 0;

	actions[1].sem_num = 0;
	actions[1].sem_flg = SEM_UNDO;
	actions[1].sem_op = 1;

	if( semop(semset_id,actions,2) == -1 ){
		perror("semop:");
		exit(1);
	}	
}

void release_lock(int semset_id){

	struct  sembuf actions[1];

	actions[0].sem_num = 0;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = -1;

	if( semop(semset_id,actions,1) == -1 ){
		perror("semop:");
		exit(1);
	}
}
