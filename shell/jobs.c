#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jobs.h"

struct job_element {
	int jid;
	pid_t pid;
	process_state_t state;
	char *command;
	struct job_element *next;
};
typedef struct job_element job_element_t;

//head is the head of the list
//current is the current element being iterated over
struct job_list {
	job_element_t *head;
	job_element_t *current;
};

/* initializes job list, returns pointer */
job_list_t *init_job_list(){
	job_list_t *job_list = (job_list_t *) malloc(sizeof(job_list_t));
	job_list->head = NULL;
	job_list->current = NULL;
	return job_list;
}

/* 
 * cleans up jobs list
 * Note: this function will free the job_list pointer
 * DO NOT use the pointer after this function is called
 */
void cleanup_job_list(job_list_t *job_list){
	if(job_list == NULL){
		return;
	}
	
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		job_element_t *nextElement = currElement->next;
		
		//free char*'s
		if(currElement->state != NULL){
			free(currElement->state);
			currElement->state = NULL;
		}
		if(currElement->command != NULL){
			free(currElement->command);
			currElement->command = NULL;
		}
		free(currElement);
		currElement = NULL;
		
		currElement = nextElement;
	}
	
	job_list->head = NULL;
	job_list->current = NULL;
	
	free(job_list);
}

/* adds new job to list, returns 0 on success, -1 on failure */
int add_job(job_list_t *job_list, int jid, pid_t pid, process_state_t state, char *command){
	if(job_list == NULL || state == NULL || command == NULL){
		return -1;
	}
	
	job_element_t *newJob = (job_element_t *) malloc(sizeof(job_element_t));
	newJob->jid = jid;
	newJob->pid = pid;
	
	//allocate new char*'s and copy buffers in to protect our code
	newJob->state = (char *) malloc(sizeof(char) * (strlen(state) + 1));
	memcpy(newJob->state, state, strlen(state) + 1);
	newJob->command = (char *) malloc(sizeof(char) * (strlen(command) + 1));
	memcpy(newJob->command, command, strlen(command) + 1);
	newJob->next = NULL;
	
	if(job_list->head == NULL){
		//add to head
		job_list->head = newJob;
		job_list->current = newJob;
	} else {
		//add to tail
		job_element_t *currElement = job_list->head;
		while(currElement->next != NULL){
			currElement = currElement->next;
		}
		currElement->next = newJob;
	}
	
	return 0;
}

/* removes job from list, given job's JID, returns 0 on success, -1 on failure */
int remove_job_jid(job_list_t *job_list, int jid){
	if(job_list == NULL){
		return -1;
	}
	
	job_element_t *prevElement = NULL;
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		if(currElement->jid == jid){
			if(prevElement != NULL){
				prevElement->next = currElement->next;
			}
			if(job_list->head == currElement){
				job_list->head = currElement->next;
			}
			if(job_list->current == currElement){
				job_list->current = currElement->next;
			}
			
			//free char*'s
			if(currElement->state != NULL){
				free(currElement->state);
				currElement->state = NULL;
			}
			if(currElement->command != NULL){
				free(currElement->command);
				currElement->command = NULL;
			}
			free(currElement);
			currElement = NULL;
			
			return 0;
		}
		
		prevElement = currElement;
		currElement = currElement->next;
	}
	
	return -1;
}

/* removes job from list, given job's PID, returns 0 on success, -1 on failure */
int remove_job_pid(job_list_t *job_list, pid_t pid){
	if(job_list == NULL){
		return -1;
	}
	
	job_element_t *prevElement = NULL;
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		if(currElement->pid == pid){
			if(prevElement != NULL){
				prevElement->next = currElement->next;
			}
			if(job_list->head == currElement){
				job_list->head = currElement->next;
			}
			if(job_list->current == currElement){
				job_list->current = currElement->next;
			}
			
			//free char*'s
			if(currElement->state != NULL){
				free(currElement->state);
				currElement->state = NULL;
			}
			if(currElement->command != NULL){
				free(currElement->command);
				currElement->command = NULL;
			}
			free(currElement);
			currElement = NULL;
			
			return 0;
		}
		
		prevElement = currElement;
		currElement = currElement->next;
	}
	
	return -1;
}

/* updates job's state, given job's JID, returns 0 on success, -1 on failure */
int update_job_jid(job_list_t *job_list, int jid, process_state_t state){
	if(job_list == NULL){
		return -1;
	}
	
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		if(currElement->jid == jid){
			//free char * and allocate new char * to protect our code
			if(currElement->state != NULL){
				free(currElement->state);
				currElement->state = NULL;
			}
			currElement->state = (char *) malloc(sizeof(char) * (strlen(state) + 1));
			memcpy(currElement->state, state, strlen(state) + 1);
			return 0;
		}
		
		currElement = currElement->next;
	}
	
	return -1;
}

/* updates job's state, given job's PID, returns 0 on success, -1 on failure */
int update_job_pid(job_list_t *job_list, pid_t pid, process_state_t state){
	if(job_list == NULL){
		return -1;
	}
	
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		if(currElement->pid == pid){
			//free char * and allocate new char * to protect our code
			if(currElement->state != NULL){
				free(currElement->state);
				currElement->state = NULL;
			}
			currElement->state = (char *) malloc(sizeof(char) * (strlen(state) + 1));
			memcpy(currElement->state, state, strlen(state) + 1);
			return 0;
		}
		
		currElement = currElement->next;
	}
	
	return -1;
}

/* gets PID of job, given job's JID, returns PID on success, -1 on failure */
pid_t get_job_pid(job_list_t *job_list, int jid){
	if(job_list == NULL){
		return -1;
	}
	
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		if(currElement->jid == jid){
			return currElement->pid;
		}
		
		currElement = currElement->next;
	}
	
	return -1;
}

/* gets JID of job, given job's PID, returns JID on success, -1 on failure */
int get_job_jid(job_list_t *job_list, pid_t pid){
	if(job_list == NULL){
		return -1;
	}
	
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		if(currElement->pid == pid){
			return currElement->jid;
		}
		
		currElement = currElement->next;
	}
	
	return -1;
}

/* 
 * gets next PID in list
 * call this in a loop to get the PID of the next job in the list
 * returns the PID if there is one, -1 if the end of the list has been reached,
 * after which it will start at the head of the list again
 */
pid_t get_next_pid(job_list_t *job_list){
	if(job_list == NULL){
		return -1;
	}
	
	if(job_list->current == NULL){
		job_list->current = job_list->head;
		return -1;
	} else {
		pid_t pid = job_list->current->pid;
		job_list->current = job_list->current->next;
		return pid;
	}
}

/* jobs command, prints out the jobs list */
void jobs(job_list_t *job_list){
	if(job_list == NULL){
		return;
	}
	
	job_element_t *currElement = job_list->head;
	while(currElement != NULL){
		char buffer[1024];
		memset(buffer, 0, 1024);
		sprintf(buffer, "[%d] (%d) %s %s\n", currElement->jid, currElement->pid, currElement->state, currElement->command);
		
		write(1, buffer, 1024);
		
		currElement = currElement->next;
	}
}
