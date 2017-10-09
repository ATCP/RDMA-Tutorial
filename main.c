#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>


#include "debug.h"
#include "config.h"
#include "ib.h"
#include "setup_ib.h"
#include "client.h"
#include "server.h"
#include "shmalloc.h"

FILE	*log_fp	     = NULL;

int	init_env    ();
void	destroy_env ();

int main (int argc, char *argv[])
{
    int	ret = 0;

    //for shared memory
    char c;
    //int shmid;
    key_t key;
    char *s;
    key = 5678;

    if (argc == 3) {
	config_info.is_server   = false;
	config_info.server_name = argv[1];
	config_info.sock_port   = argv[2];
    } else if (argc == 2) {
	if (!strcmp(argv[1], "55889")) { 
	    fprintf(stderr, "%s\n", argv[1]);
	    if ((shmid = shmget(key, SHMSZ, 0777)) < 0) {
	        perror("shmget");
		exit(1);
	    }
	    
	    if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
	        perror("shmat");
		exit(1);
	    }
	   
	    /*
	    for (s = shm; *s != NULL; s++)
	        putchar(*s);
	    putchar('\n');
	    */
	    
	} else {
	    shmdt(shm);
	    return -1;
	}
			
	config_info.is_server = true;
	config_info.sock_port = argv[1];
    } else {
	//create the segment
	if ((shmid = shmget(key, SHMSZ, 0777 | IPC_CREAT | IPC_EXCL)) < 0) {
	        perror("shmget");
		exit(1);
	}
	
	//attach the segment to data space
	if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
	        perror("shmat");
		exit(1);
	}

	/*
	memset(shm, 0, SHMSZ);
	
	char cmd[80];
	strcpy (cmd, argv[0]);
	strcat (cmd, " 55889 ");

	fprintf(stderr, "%s\n", cmd);

	system(cmd);
	*/

	/*
	struct IBRes *res = (struct IBRes *)shm;	    
	while (1) {
	   if (res->ib_buf_size) {
	      fprintf(stderr, "%d\n", res->ib_buf_size);
	      break;
	   }
	   sleep(1);
	   break;
	}*/
	
	double *ptr;
	size_t dbl_size = 1024;

	// Regular use of malloc
	printf("Regular use of malloc\n");
	ptr = (double *) shmalloc(3, &dbl_size, shm, SHMSZ);
	
	while(1);
	//shmctl(shmid, IPC_RMID, NULL);
	//shmdt(shm);


	printf ("Server: %s sock_port\n", argv[0]);
	printf ("Client: %s server_name sock_port\n", argv[0]);
	return 0;
    }    

    config_info.msg_size         = 64; 
    config_info.num_concurr_msgs = 1;

    ret = init_env ();
    check (ret == 0, "Failed to init env");

    ret = setup_ib ();
    check (ret == 0, "Failed to setup IB");

    if (config_info.is_server) {
        ret = run_server ();
    } else {
        ret = run_client ();
    }
    check (ret == 0, "Failed to run workload");

 error:
    close_ib_connection ();
    destroy_env         ();
    return ret;
}    

int init_env ()
{
    if (config_info.is_server) {
	log_fp = fopen ("server.log", "w");
    } else {
	log_fp = fopen ("client.log", "w");
    }
    check (log_fp != NULL, "Failed to open log file");

    log (LOG_HEADER, "IB Echo Server");
    print_config_info ();

    return 0;
 error:
    return -1;
}

void destroy_env ()
{
    log (LOG_HEADER, "Run Finished");
    if (log_fp != NULL) {
        fclose (log_fp);
    }
}
