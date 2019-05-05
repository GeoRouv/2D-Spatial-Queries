#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc,char** argv){
    int z=0,WorkersCount=0,WorkersCount_flag=0;
    char *inputfile = NULL,*TempDir = NULL;

    for(z=0; z<argc; z++){	        //Getting tokens 
        if(!(strcmp(argv[z],"-i"))){
            inputfile = malloc(strlen(argv[z+1])+1);
			strcpy(inputfile,argv[z+1]);
        } 
        else if(!(strcmp(argv[z],"-w"))){
            WorkersCount = atoi(argv[z+1]);
            WorkersCount_flag = 1; 
        }	
        else if(!(strcmp(argv[z],"-d"))){
            TempDir = malloc(strlen(argv[z+1])+1);
			strcpy(TempDir,argv[z+1]);
        }
    }

    if(WorkersCount_flag == 0){     //Input error handling
        printf("No workers token given\n");
        return 0;
    }
    else if(inputfile == NULL){
        printf("No inputfile given\n");
        return 0;
    }
    else if(TempDir == NULL){
        printf("No Temporary Directory given\n");
        return 0;
    }


    FILE *fp;
    fp = fopen(inputfile, "r");				//Reading points from a text file...   
    
    int SizeL=0,numOfrecords=0;
    fseek(fp, 0 , SEEK_END);
    SizeL = ftell(fp);
    numOfrecords = SizeL/(2*sizeof(float)); //Number of input coordinates(records) 
	fclose(fp);

	struct stat st = {0}; //Creating a new directory(if it doesn't exists already) according to "TempDir"
	if (stat(TempDir, &st) == -1) {
		mkdir(TempDir, 0700);
	}
					
/////////////////////////////////////////////////////////CLI of MASTER Process//////////////////////////////////////////////////////////////

    printf("Entering CLI mode.Type your commands:\n");
    char *buffer;
	int tread,i,k,counter,position,NumStrings;
	long unsigned int len;
    int flag1=0;
	int commands_counter=0;
	//~~~~~~~~~~~~//
	char **strings;
	char ***tmp;
	int y,rows;
	char *token;
	char **colours;
	//~~~~~~~~~~~~//
	int *pid_array;
	int pid,procNum;
	int PointsToRead;
	int offset;		
	char s1[20],s2[20],s3[50],tmp_buf[50];    //Temporary Buffers

	int* fds;
	int retval;
	int j;
	char last_buffer[100000]; 
	FILE *targetfile;
	int n;
	int gnuplot_pid;
	char buffer90[100];
	
    while(1){       
        buffer = NULL;
        tread=0,i=0,k=0,counter=0,position=0;
        len=0;

        printf(">>");
        tread = getline(&buffer, &len, stdin); //Getting command
		if(!strcmp(buffer,"exit\n")) return 0; //"Exit" command comparison
        NumStrings = 10;
        strings = malloc(sizeof(char*) * NumStrings); //2D array for storing command strings
    
        if (-1 != tread){            //if Given any command , proceed
           for(i=0;i<= strlen(buffer);i++){
                if(buffer[i]==';' || buffer[i]==','){ //if end of command store it
                    if(buffer[i]!=',') flag1=1;       //Flag to check whether command ends with ';'
                    strings[counter]=malloc(strlen(buffer));
                    if(counter==0) strncpy(strings[counter],buffer,i);
                    else strncpy(strings[counter],buffer+position+1,i-position-1);
                    counter+=1;     //number of commands of input
                    position=i;
                }
           }
           if(!flag1){
               printf("Program Terminated!Please end the given command with ';'\n");
               return 0;
           }  
        }
        else printf("No line read...\n");

	///Time to tokenize strings//
      
        					
		rows=10;
        tmp=malloc(sizeof(char**)*k);	//Allocating space for 2D array which will hold all commands given
        for(k=0;k<counter;k++){
            tmp[k]=malloc(sizeof(char*)*rows);
            for(y=0;y<rows;y++){
                tmp[k][y]=malloc(sizeof(char)*100);
            }
        }

        
        for(k=0;k<counter;k++){
            /* get the first token */
            token = strtok(strings[k]," ,");
            /* walk through other tokens */
            y=0;
            while( token != NULL ){
		        strcpy(tmp[k][y],token);    //Insert command tokens in the 2D array
		        y+=1;
		        token = strtok(NULL, " ,");
            }
        }
		free(token);

		for(k=0;k<counter;k++)	//Freeing "strings" array
			free(strings[k]);
		free(strings);
	 
		colours=malloc(sizeof(char*)*counter);
		for(k=0;k<counter;k++){		//Getting colour for every command given and storing it to "colours" array
			
			if(!strcmp(tmp[k][0],"circle\0") || !strcmp(tmp[k][0],"square\0")){
				colours[k]=malloc(sizeof(char)*(strlen(tmp[k][4])+1));
				strcpy(colours[k],tmp[k][4]); 
			}
			else{
				colours[k]=malloc(sizeof(char)*(strlen(tmp[k][5])+1));
				strcpy(colours[k],tmp[k][5]); 
			}
		}		
			
		

//////////////////////////////////////////////////     
        
        pid=0,procNum=0;
        pid_array=malloc(counter*sizeof(int));

        for(procNum = 0; procNum < counter; procNum++){
				
			pid = fork();        
			if(pid>0){
				pid_array[procNum]=pid; //keeps track of handlers id's     
				wait(NULL);         //MASTER process waiting for handlers to finish
			}
            else if(pid == 0){
                // child specific stuff. procNum is set to the "child number"
                printf("I am handler |%d|\n",procNum);

                PointsToRead = numOfrecords/WorkersCount;
                if(numOfrecords % WorkersCount != 0) PointsToRead+=1; //Assign more work for all workers,later we will subtract work from the last one
                printf("Records:|%d| , Workers:|%d|\n\n",numOfrecords,WorkersCount);
                offset=0;   //offset=0 for the first worker
   				
				fds = malloc(WorkersCount*sizeof(int)); //File descriptors list

				////////////////////Worker Part//////////////////////////////
								
                for(j=0;j<WorkersCount;j++){				
					sprintf(tmp_buf,"./%s/%d_w%d.fifo",TempDir,getpid(),j);
					retval = mkfifo(tmp_buf,0666);	//Creating FIFO file
					
					if (( retval  ==  -1) && (errno !=  EEXIST )){	//Error handler
						perror("Error creating the named pipe");
						exit (1);
					}
                    if(j!=0){   					//Assign the according offset if its not the first worker 
                    	if(numOfrecords % WorkersCount !=0 && j==WorkersCount-1) PointsToRead-=WorkersCount-(numOfrecords % WorkersCount); //If no equal work among workers then the last one does less
                   	    offset=j*PointsToRead*sizeof(float);
                    } 
                    sprintf(s2 , "%d" ,PointsToRead); //Converting ints offset,PointsToRead to strings
                    sprintf(s1 , "%d" ,offset);

					pid=fork();
					if(pid>0){
						fds[j]=open(tmp_buf, O_RDONLY | O_NONBLOCK); //Opens fifo for reading if is at parent
						wait(NULL);
					}
                    if(pid==0){		//Executing Utilities..
                        printf("I am worker %d for handler |%d|\n",j,procNum);
						
                        if(!strcmp(tmp[procNum][0],"circle")){	
                            execl("./circle","-i",inputfile,"-o",tmp_buf,"-a",tmp[procNum][1],tmp[procNum][2],tmp[procNum][3],"-n",s2,"-f",s1,(char *)NULL);
                        }
                        else if(!strcmp(tmp[procNum][0],"semicircle")){
                            execl("./semicircle","-i",inputfile,"-o",tmp_buf,"-a",tmp[procNum][1],tmp[procNum][2],tmp[procNum][3],tmp[procNum][4],"-n",s2,"-f",s1,(char *)NULL);
                        }
                        else if(!strcmp(tmp[procNum][0],"ring")){
                            execl("./ring","-i",inputfile,"-o",tmp_buf,"-a",tmp[procNum][1],tmp[procNum][2],tmp[procNum][3],tmp[procNum][4],"-n",s2,"-f",s1,(char *)NULL);
                        }
                        else if(!strcmp(tmp[procNum][0],"square")){
                            execl("./square","-i",inputfile,"-o",tmp_buf,"-a",tmp[procNum][1],tmp[procNum][2],tmp[procNum][3],"-n",s2,"-f",s1,(char *)NULL);
                        }
                        else if(!strcmp(tmp[procNum][0],"ellipse")){
                            execl("./ellipse","-i",inputfile,"-o",tmp_buf,"-a",tmp[procNum][1],tmp[procNum][2],tmp[procNum][3],tmp[procNum][4],"-n",s2,"-f",s1,(char *)NULL);
                        }     
                        printf("Error in executing utility\n");
                        return 0;  
                    }
                }//End of workers loop
				
			
				sprintf(s3 ,"./%s/%d.out",TempDir,getpid()); 
				targetfile=fopen(s3,"w");

				for(j=0;j<WorkersCount;j++){	//Transferring data from opened FIFOs to PIDhandler.out				
						while((n=read(fds[j],last_buffer,99999))){
							last_buffer[n]='\0';
							fprintf(targetfile,"%s",last_buffer);	
						}
				}
                fclose(targetfile);
				free(fds);
				

                printf("Handler %d terminated\n",procNum);
                return 0;
            }
            
        }//End of commands loops
		while(wait(NULL) > 0); //MASTER waits all handlers to finish

		//////////////////////////////////////////////////Gnu Plot Script///////////////////////////////////////////////////////
				
		sprintf(buffer90 , "%s/%d_script.gnuplot" , TempDir , commands_counter);
        FILE* script = fopen(buffer90 , "w");
        fprintf(script , "set terminal png\n");
        fprintf(script , "set size ratio -1\n");
        fprintf(script , "set output \"%d_image.png\"\n" ,commands_counter);
        fprintf(script , "plot \\\n");
	
        for(i = 0; i < counter - 1; i++){
            fprintf(script , "\"%s/%d.out\" notitle with points pointsize 0.5 linecolor rgb \"%s\",\\\n" , TempDir , pid_array[i] , colours[i]);
        }
        fprintf(script , "\"%s/%d.out\" notitle with points pointsize 0.5 linecolor rgb \"%s\"" , TempDir , pid_array[counter-1] , colours[counter-1]);
 
        fclose(script);
        commands_counter++;
 
        gnuplot_pid = fork();
        if(gnuplot_pid < 0){printf("Fork failed!\n"); exit(1);}
        else if(gnuplot_pid == 0)
        {
            printf("Executing script: %s\n" , buffer90);
            execl("/usr/bin/gnuplot" , "gnuplot" , buffer90 , (char*) NULL);
            printf("Execl failed!\n"); exit(1);
        }
        else wait(NULL);
		
		free(pid_array);
		for(i = 0; i < counter; i++){
			free(colours[i]);	
		}
		free(colours);
	
	}
/////////////////////////End of CLI mode/////////////////////////////
    free(buffer);
    
   
 return 0;
}




