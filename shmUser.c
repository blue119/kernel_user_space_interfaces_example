#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>

int main () {
	key_t schluessel;             /* Schluessel */
	int shmid;                    /* Shared Memory ID */
	char* seg;                    /* Adresse zum Einhaengen */
	int pid;                      /* PID des Sohnes */

	/* Shared Memory segment belegen */
	/* Schluessel Null = IPC_PRIVATE, d.h. es  */
	/* wird auf jeden Fall ein neues Segment erzeugt */
	key_t key = ftok("/home/arkeller/testbla", 1);
	printf("key generated %i\n", key);
	key = 17236104;
	perror("ftok");
	shmid = shmget (key, 40, IPC_CREAT|0600);
	if (shmid==-1) {
		perror("shmget");
		exit(1);
	}
	printf("Shared memory erzeugt\n");
	/* jetzt Prozess starten, der hinein schreibt */
	pid=fork();
	if (pid==0) {
		printf("Sohn\n");
		seg = (char *)shmat(shmid, NULL, 0);
		if (seg==(void *)-1) {
			perror("shmat");
			exit(1);
		}
		printf("Ein Zeichen eingeben:");
		scanf("%1c",seg);
		shmdt(seg);
	}
	else {
		wait(pid);
		printf("Sohn beendet..");
		printf ("Lese shared memory..\n");
		seg = (char *)shmat(shmid,0,0);
		printf ("Inhalt: %c\n",*seg);
		sleep(20);
		printf ("Lese shared memory..\n");
		printf ("Inhalt: %c\n",*seg);

		shmdt(seg);
		/* jetzt Segment entfernen */
		shmctl(shmid,IPC_RMID,0);
	}
	return 1;

}
