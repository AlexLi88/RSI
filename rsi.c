#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <signal.h>    
#include <errno.h>      
#include <time.h>

#define MAX_BUFFER_LENGTH 256
#define DEBUG 0

//The command which accpeted in this shell
//1.  ls
//2.  cd
//3.  bg
//4.  bglist


int command_number = 0; //store the number of command
int bg_number = 0; //store the number of background jobs.
char *exe[10]; //store the back ground job in an char array without "background"
char Id_buffer[10]; //store the process id
char command_argument[10][MAX_BUFFER_LENGTH]; //store the command
char bglist[100][MAX_BUFFER_LENGTH]; //store the background list
void lsFunction(void); //ls function
void bgFunction(void); //back ground function
void getToken(char *); //tokenize the input 
void printBglist(void); //print the back ground list 

static void sigchld_handler (int signal)
{
	while (waitpid(-1, NULL, WNOHANG) > 0) {
		if (DEBUG) printf("\nSIGCHLD handler activated. Received signal %d\n", signal);
	}
}

void printWelcome()
{
	puts("\n==========================");
	puts("Welcomt to CSC RSI");
	puts("Student Name: Xiaotian Li");
	puts("Student Number: V00786924");
	puts("==========================\n");
}

void printPrompt() //print the prompt and run the command 
{
	int bailout = 0;
	
	char change[MAX_BUFFER_LENGTH];
	
	while(!bailout)
	{	
		if(DEBUG) printf("process ID is %d",getpid());
		
		char prompt[MAX_BUFFER_LENGTH] = "RSI: ";
		char cur[MAX_BUFFER_LENGTH];
		getcwd(cur,MAX_BUFFER_LENGTH);
		char* username;
		username = (char *)malloc(20*sizeof(char));
		username = getlogin();  //get the current username
		strcat(prompt,cur);
		strcat(prompt, " > ");
		char* reply = readline(prompt);
		getToken(reply);
		free(reply);
		if(!strncmp(command_argument[0],"ls",2)){
			//printf("Welcome to ls\n");
			lsFunction();
			memset(&command_argument[0], 0, sizeof(command_argument));
		}
		else if(!strcmp(command_argument[0],"exit")){
			bailout = 1;
			printf("\n=====Thanks for Using=====\n\n");
			exit(0);
		}else if(!strcmp(command_argument[0],"cd")){
			if(strcmp(command_argument[1],"")){ 
				//cd with argument
				if(!strncmp(command_argument[1],"~",1)){ 
					//cd ~ with directory follow
					if(strlen(command_argument[1]) != 1){
						int len = strlen(command_argument[1]) - 3;
						//printf("\nlength: %d\n",len);
						char temp[len];
						strncpy(temp, command_argument[1] + 2,len);
						strcpy(change,"/home/");
						strcat(change,username);
						strcat(change,"/");
						strcat(change,temp);
						//printf("\nChange : %s\n",change);
					}else{
						//cd ~ without directory followed
						strcpy(change,"/home/");
						strcat(change,username);
						//strcat(change,"/");
						//strcat(change,temp);
					}
				}else{
					strcpy(change,command_argument[1]);
				}
				if(chdir(change) == 0){
					getcwd(cur,MAX_BUFFER_LENGTH);
					if(DEBUG)
					printf("\nchange dir %s\n",cur);
				}else{
					getcwd(cur,MAX_BUFFER_LENGTH);
					printf("\nNo such file or directory\n");
				}
			}else{ // if cd without any argument. 
				strcpy(change,"/home/");
				strcat(change,username);
				if(chdir(change) == 0){
					getcwd(cur,MAX_BUFFER_LENGTH);
					if(DEBUG) printf("\nchange dir %s\n",cur);
				}
			}
			memset(&command_argument[0], 0, sizeof(command_argument));
		}else if(!strcmp(command_argument[0], "bg")){ //run the function in background. 
			bgFunction();
			memset(&command_argument[0], 0, sizeof(command_argument));
		}else if(!strcmp(command_argument[0], "bglist")){ //run bglist. 
			printBglist();
			memset(&command_argument[0], 0, sizeof(command_argument));
		}else if(strcmp(command_argument[0],"ls") && strcmp(command_argument[0],"cd") && strcmp(command_argument[0],"bg") && strcmp(command_argument[0], "bglist")){
			if(command_number != 0){
				printf("%s: command not found\n",command_argument[0]);
			}else{
				continue;
			}
			memset(&command_argument[0], 0, sizeof(command_argument));
		}

		
	}
	
	
}


void lsFunction()
{
	pid_t fpid;
	fpid = fork();
	if(fpid < 0){
		fprintf(stderr,"Fork Failed");
		exit(-1);
	}else if (fpid == 0){
		
		if(DEBUG) printf("\nLS child process id%d\n",getpid());
		if(!strcmp(command_argument[1],"")){ //accept one argument 
			execlp("/bin/ls","ls",NULL);
		}else if(strcmp(command_argument[1],"")){ //accept two arguments
			if(strcmp(command_argument[1],"") && strcmp(command_argument[2],"")){ //accept three arguments
				execlp("/bin/ls","ls",command_argument[1],command_argument[2],NULL);
			}
			execlp("/bin/ls","ls",command_argument[1],NULL);
		}
		
		exit(5);
	}else{
		if(DEBUG) printf("\nLS father process id%d\n",getpid());
		wait(NULL);
	}
}

void bgFunction(){
	int x;
	int status;
	int retVal;
	pid_t cpid;
	char cur[MAX_BUFFER_LENGTH];
	
	for(x = 0; x < command_number-1; x++){
		exe[x] = (char*)malloc(sizeof(char)* 20);
		strcpy(exe[x], command_argument[x+1]);
		exe[x+1] = NULL;
	}
	if(DEBUG){
		printf("\nexe 0 is %s",exe[0]);
		printf("\nexe 1 is %s",exe[1]);
		printf("\nexe 2 is %s",exe[2]);
	}
	cpid = fork();
	if(cpid > 0){ //if cpid > 0 it means that this is the child process number. Save it to the Id_buffer and back ground list. 
		sprintf(Id_buffer, "%d",cpid);
		strcpy(bglist[bg_number],Id_buffer);
		memset(Id_buffer, 0, sizeof(Id_buffer));
	}

	if(cpid == 0)
	{
		if(DEBUG) printf("\n The pid of the child process is: %d\n", getpid());
		
		execvp(exe[0], exe);
		//exit(5);
	}else if(cpid > 0)
	{
		if(DEBUG) printf("\n The pid of the parent process is: %d\n", getpid());
		retVal = waitpid(cpid, &status, WNOHANG);
		if(DEBUG) printf("the return value is %d", retVal);
		getcwd(cur,MAX_BUFFER_LENGTH);	
		strcat(bglist[bg_number], " ");
		strcat(bglist[bg_number], cur);
		strcat(bglist[bg_number],"/");
		strcat(bglist[bg_number],exe[0]);
		bg_number++;
		if(DEBUG) {
			printf("\nbglist 0 :%s\n",bglist[0]);
			printf("\nbglist 1 :%s\n",bglist[1]);
			printf("\nbglist 2 :%s\n",bglist[2]);
		}
	}
	else
	{
		perror("\nFail to create  a new process.\n");
	}
	
}

void printBglist(void){
	int i ;
	if(bg_number == 0){
		printf("There is no background tasks\n");
	}
	for(i = 0; i < bg_number; i++){
		printf("%s\n", bglist[i]);
	}
}
void getToken(char *reply)
{
	command_number = 0;
	char seps[] = " ";
	char *token;
	int i = 0;

	token = strtok(reply,seps);
	while(token != NULL){
		strcpy(command_argument[i],token);
		i++;
		token = strtok(NULL,seps);
		command_number++;
	}
	if(DEBUG){
		printf("\ncommand argument, %s\n",command_argument[0]);
		printf("\ncommand argument2, %s\n",command_argument[1]);
		printf("\ncommand argument3, %s\n",command_argument[2]);
		printf("\ncommand number, %d", command_number);
	}
}

int main()
{
	struct sigaction action;
	memset(&action, '\0', sizeof(action));

	action.sa_handler = sigchld_handler;
	if(sigaction(SIGCHLD, &action,0)){
		perror ("sigaction");
		return 1;
	}

	printWelcome();
	printPrompt();
	
	return 0;
}
