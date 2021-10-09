/*-------------------------------------------------------*/
/* cs345sh.c                                             */
/* Make my Own CShell                      				 */
/* Philippos Sophroniou__csd3887__HY345_Assigment_1      */
/*-------------------------------------------------------*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#include  <signal.h>
#include <stdbool.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>
#define TRUE 1
#define FALSE 0

#define SIZE 5000
#define RESET   "\033[0m"
#define GREEN   "\033[32m"      /* Green */
/*------------------------------------------------------ Global Variables ----------------------------------------------------------*/
char * user_name, * user_dir;
char buff[SIZE];
char  user_input[100];
char *tokenize_array[100]; // tokenize_array will fill with all tokens for every user_input
int count_tokenize_words=0;
int megethos2;
int is_deamon;
int megethos;
char *array[100]; // array will fill with all tokens until <<, >, < found...e.g user_input = ls -l then array[0] = ls and array[1] = -l
				 // e.g user_input = ls -l >> test.txt then array[0] = ls and array[1] = -l array[2] = NULL
int m;
void INThandler(int); // Ctr+C
_Bool fill_array = TRUE;


/*---------------------------------------------------------- Functions -------------------------------------------------------------*/
/*Print Prompt*/
void print_prompt(){
	user_name = getlogin(); // Get the user who is logged in
	user_dir = getcwd(buff,SIZE); // Get user current working directory
	if (user_name == "" && user_dir == ""){ // Error check
		printf("No User found, please try again!!!\n");
	}
	printf( GREEN "%s@cs345sh%s$ " RESET ,user_name,user_dir); // Print user's infos in prompt
}

/* Exit function */
void exit_function(){
	printf("---------- Exit Cshell ---------\n");
	exit(EXIT_SUCCESS); 
}

/* Cd Function */
void cd_function(char *user_input){ 
	chdir(&user_input[3]); // Go to address of directory that starts in 3rd position of user input (after cd space)
}

/* Cd Home -- Go to Home directory of User */
void cd_home(char *user_input){
	chdir(getenv("HOME")); // Go to home directory of user
}

/* Split input in words using space */
int token_ize_input(char user_input[]){
    int i;
    char *token = strtok(user_input, " ");  // Get the first token before space
    
    // Keep printing tokens while one of the 
    // delimiters present in str[]. 
    i=0; // Position of tokenize_array
	m =0 ; // Position of array
    count_tokenize_words = 0; // Count tokens
    fill_array = TRUE;
    while (token!=NULL) // Continue untill there no more tokens
    { 
        tokenize_array[i] = token; // First token go to first (0) position of tokenize_array
		if (strcmp(tokenize_array[i], ">>") == 0 || strcmp(tokenize_array[i], ">") == 0 || strcmp(tokenize_array[i], "<") == 0){
			fill_array = FALSE; // Set flag
		}
		if (fill_array){ // If true fill array
			array[m] = tokenize_array[i]; // array will have any token before ">>", ">", "<"
			m++; // Increase position of array
		}
        count_tokenize_words++; // Count how many tokens user gives including ">>", ">", "<"
        i++; // Increase position of tokenize_array
        token = strtok(NULL, " "); // Separate them by space " "
    }    
    return EXIT_SUCCESS;
} 

// Check if Ctr+C was pressed and exit
void  INThandler(int sig){
    signal(sig, SIG_IGN);
    printf("\n-----> You have pressed Ctl+C <-----\n");
    exit_function();
}


/*--------------------------------------------------------- Main Function ----------------------------------------------------------*/
int main(){
	signal(SIGINT, INThandler); // Cath Ctr+C
	FILE *open_file;
	int i;
	pid_t pid, fg_pid;
	int change_stdout=0;
	int change_stdin=0;
	int find = 0;
	char *env_args[] = { (char*)0 };
	int screen_stdout;
	int screen_stdin;			
	int print_in_file = 0;
	char enter_key;
	int deamon_process = 0;
	int status;
	_Bool put_job_in_foreground = FALSE;

	//rl_bind_key('\t', rl_complete);

	while(TRUE){ // Infinte Loop until user type exit or press Ctr+C
		// Initialize user_input
		i = 0;
		while(i<100){
 			user_input[i]='\0';
 			i++;
		}	

		// Initialize tokenize_array and array where tokens will be
		for(i=0; i<100; i++){
			tokenize_array[i] = NULL ;
			array[i] = NULL ;
		}

		print_prompt();	// Print Prompt

		fgets(user_input,sizeof user_input,stdin);	// Get input from user

		//add_history(user_input);

		// Check if user pressed just enter and continue
		if (user_input[0] == 0x0A){ // ASCII Code of Enter Key in Hexadecimal. 
			continue; // If user pressed only enter continue
		}

		// Background Process
		if ( user_input[strlen(user_input) - 2] == '&'	){ // If last character was '&' then process must be deamon
			user_input[strlen(user_input) - 1] = '\0';
			deamon_process = 1; // Set flag for deamon processes
		}else{
			deamon_process = 0; // Set flag for non deamon processes
		}

		megethos = strlen(user_input);
		user_input[(megethos-1)] = '\0';

		token_ize_input(user_input);	// Split input into words using space

		// Exit
		if (strcmp(array[0], "exit") == 0){ // if user typed exit call exit_function
			exit_function();
		}

		// Foreground Process
		if (strcmp(array[0], "fg") == 0){
			//printf("Process which must run in foreground: %d\n", fg_pid);
			//setpgid(fg_pid, 2);
			tcsetpgrp(2, fg_pid); // Set the  process with pid = fg_pid in foreground
			
		} 

		// Cd
		if (strcmp(tokenize_array[0], "cd") == 0){
			if(count_tokenize_words <= 1 || strcmp(tokenize_array[1],"~") == 0){ // if count_tokenize_words <=1 then input was only cd 	
				cd_home(user_input); // so go to home folder
			}
			else{
				cd_function(user_input); // else go to any folder user wants
			}
		}

		find =0;
		while(find<count_tokenize_words){ // Search the tokenize_array for specific operators ">>",">","<"
			// If operator ">>" found then open or create the file given and replace all the text with stdout 
        	if (strcmp(tokenize_array[find], ">>") == 0){
        		print_in_file = 1;	// To know what exec() to use 
        		printf("File must be opened: %s\n",tokenize_array[find + 1] );
				open_file = fopen(tokenize_array[find+1],"w+");	// Open the file or create it and replace data 
				if(open_file == NULL){	// Some error
					printf("Something went wrong file did not created or opened!!!\n");
					return 1;
				}
				screen_stdout = dup(1); // save current stdout
				close(1); // close stdout - Stop Printing on screen
				dup2(fileno(open_file),1); // redirect output to open_file - Start printing to that file
    			change_stdout = 1; // Flag to know if i changed stdout        		
        	}
        	// If operator ">" found then open or create the file given and append stdout at the end of the file
        	if (strcmp(tokenize_array[find], ">") == 0){
        		print_in_file = 1;	// To know what exec() to use 
        		printf("File must be opened: %s\n",tokenize_array[find + 1] );
				open_file = fopen(tokenize_array[find+1],"a");	// Open the file or create it and write there - append data
				if(open_file == NULL){	// Some error
					printf("Something went wrong file did not created or opened!!!\n");
					return 1;
				}
				screen_stdout = dup(1); // save current stdout
				close(1); // close stdout - Stop Printing on screen
				dup2(fileno(open_file),1); // redirect output to open_file - Start printing to that file
    			change_stdout = 1; // Flag to know if i changed stdout        		
        	}

        	// If operator "<" found then open the file given and read the command from there.
        	if (strcmp(tokenize_array[find], "<") == 0){
        		print_in_file = 1;	// To know what exec() to use 
        		printf("File must be opened: %s\n",tokenize_array[find + 1] );
				open_file = fopen(tokenize_array[find+1],"r");	// Open the file and read
				if(open_file == NULL){	// Some error
					printf("Something went wrong file did not opened!!!\n");
					return 1;
				}
				screen_stdin = dup(0); // save current stdin
				close(0); // close stdin - Stop reading from screen
				dup2(fileno(open_file),0); // redirect input to open_file - Start from that file
    			change_stdin = 1; // Flag to know if i changed stdin        			
        	}
        	find++; // Increase find
    	}
			
		// Other commands needs fork
		pid = fork();	// Create fork
		fg_pid = pid;
       	if (pid < 0) {
       		printf("Something went wrong with fork!!!\n");
            //waitpid(pid,NULL,0);	//Wait for child to finsh
			//exit_function();
        }
        if ( pid == 0) {
        	execvp(array[0], array); // Get commands and parameters and execute command
        }
        else if (deamon_process == 0){ // Wait for proccess to finish else means it is deamon process
        	waitpid(pid,NULL,0);
        }else{
        	printf("Deamon Process\nName: %s        PID: %d\n",user_input,pid ); // Deamon Process
        	fg_pid = pid;
        	deamon_process == 0;
        }

		// Restore stdout
        if(change_stdout == 1){	// If stdout was changed from screen then...
			dup2(screen_stdout,1); //redirect output to screen_stdout (restore-start printing on screen)
			change_stdout = 0; // Set flag to zero 
			fclose(open_file); // Close the file that was opened
		}

		// Restore stdin
		if(change_stdin == 1){	// If stdin was changed from screen then...
			dup2(screen_stdin,0); //redirect input to screen_stdin (restore-start reading from screen)
			change_stdin = 0; // Set flag to zero 
			fclose(open_file); // Close the file that was opened
		}
		//free(user_input);
	}
	return 0;
}

/*----------------------------------------------------------- END -----------------------------------------------------------------*/

