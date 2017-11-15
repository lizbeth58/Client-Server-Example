#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>

void lock(int semset_id);
void release(int semset_id);


int main(int argc, char** argv){

	int semset_id;
	int seg_id;
	char* memptr;

	char buffer[256];

	if( (seg_id = shmget(99,256,0777)) == -1 ){
		perror("shmget:");
		exit(1);
	}

	memptr = shmat(seg_id,NULL,0);

	if( memptr == (void*) -1 ){
		perror("shmat:");
		exit(1);
	}


	semset_id = semget(9900,2,0);

	if( semset_id == -1 ){
		perror("semset");
		exit(1);
	}

	while(1){

		printf(">");

		fgets(buffer,256,stdin);

		buffer[strlen(buffer)-1] = '\0'; 


		lock(semset_id);
		
		strcpy(memptr,buffer);

		release(semset_id);

		if( strcmp(buffer,"QUIT") == 0 ){
			exit(0);
		}
	}

	return 0;
}

void lock(int semset_id){

	struct sembuf actions[2];

	actions[0].sem_num = 0;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = 0;

	actions[1].sem_num = 1;
	actions[1].sem_flg = SEM_UNDO;
	actions[1].sem_op = 1;

	if( semop(semset_id,actions,2) == -1 ){
		perror("semop:");
		exit(1);
	}	
}

void release(int semset_id){

	struct  sembuf actions[1];

	actions[0].sem_num = 1;
	actions[0].sem_flg = SEM_UNDO;
	actions[0].sem_op = -1;

	if( semop(semset_id,actions,1) == -1 ){
		perror("semop:");
		exit(1);
	}
}