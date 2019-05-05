#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	float	x;  	
	float	y;
} MyRecord;

int main(int argc , char** argv){
    int z,offset,offset_flag=0,PointsToReadCount,PointsToReadCount_flag=0,a_flag=0;
    float r1,r2,center_x,center_y;
    char* inputfile=NULL,*outputfile=NULL;

	for(z=0; z<argc; z++){						//Getting tokens from command line
		if(!(strcmp(argv[z],"-a"))){

		    center_x = atof(argv[z+1]);
            center_y = atof(argv[z+2]);
            r1 = atof(argv[z+3]);
            r2 = atof(argv[z+4]);
            if(r1 < r2){ //if r1 < r2 swap them
                float temp;
                temp=r1;
                r1=r2;
                r2=temp;
            }
            
            a_flag=1;       //Error flag if no input given

		}
        else if(!(strcmp(argv[z],"-i"))){

            inputfile = malloc(strlen(argv[z+1])+1);
			strcpy(inputfile,argv[z+1]);

        } 
        else if(!(strcmp(argv[z],"-o"))){

            outputfile = malloc(strlen(argv[z+1])+1);
			strcpy(outputfile,argv[z+1]);

        } 
        else if(!(strcmp(argv[z],"-f"))){

            offset = atoi(argv[z+1]);
            offset_flag=1;
        } 		
        else if(!(strcmp(argv[z],"-n"))){
            PointsToReadCount = atoi(argv[z+1]);
            PointsToReadCount_flag=1;
        }				
	}

    if(a_flag==0){
        printf("Program Terminated: No input given of type '-a'\n");
        return 0;
    }
    else if(inputfile==NULL){
        printf("Program Terminated: No input given of type '-i'\n");
        return 0;
    }
    else if(outputfile==NULL){
        printf("Program Terminated: No input given of type '-o'\n");
        return 0;
    }

    printf("Ring with center(%f,%f) and r1=%f,r2=%f,offset=%d,PointsToRead=%d\n\n",center_x,center_y,r1,r2,offset,PointsToReadCount);
    
    FILE *fp,*fo;
    fp = fopen(inputfile, "r");				//Reading points from a text file...   
    fo = fopen(outputfile, "w");            //File to write to
    
    int numOfrecords, i;
    MyRecord rec;

    if(!offset_flag) offset=0;              //if offset not given , then zero     
    fseek (fp , offset , SEEK_SET);
    long pos1,pos2;
    pos1=ftell(fp);
    fseek(fp, 0 , SEEK_END);
    pos2=ftell(fp);
    numOfrecords = (int)(pos2-pos1)/sizeof(rec);

    fseek (fp , offset , SEEK_SET);

    if(PointsToReadCount_flag) numOfrecords = PointsToReadCount;
    for (i=0; i<numOfrecords ; i++) {
       	fread(&rec, sizeof(rec), 1, fp);
       	//printf("%f %f\n", rec.x, rec.y);
        if( ((rec.x - center_x)*(rec.x - center_x) + (rec.y - center_y)*(rec.y - center_y) <= r1*r1) && ((rec.x - center_x)*(rec.x - center_x) + (rec.y - center_y)*(rec.y - center_y) >= r2*r2) ){  //if point is inside the ring then print it to output file
            fprintf(fo, "%f\t%f\n",rec.x,rec.y);
        }
   	 }
    fclose(fp);
    fclose(fo);

    

    return 0;
}


