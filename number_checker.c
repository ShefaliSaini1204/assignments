#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define  MAX_LINES 100
#define SET_SIZE  5
#define MAX_LINE_LEN 256

int check_input_numbers(const char* arg, int numbers[])
{
char* copy = strdup(arg);
if(!copy)return -1;

char *token = strtok(copy,",");
int count = 0;
while(token != NULL && count < SET_SIZE)
{
int num = atoi(token);
if(num<1 || num>99)
{
free(copy);
return -1;
}
numbers[count]=num;
count++;
token = strtok(NULL,",");
}

free(copy);
if(count == SET_SIZE)
{
return 0; 
}
 else
{
return -1;
}
}

int check_line_numbers(const char* line, int numbers[])
{
int count = 0;
const char *ptr = line;
char *endptr;
while(*ptr && count < SET_SIZE)
{
while(*ptr && isspace((unsigned char)*ptr)){ptr++;}
if(!*ptr){break;}

int num = strtol(ptr, &endptr, 10);
if(ptr == endptr)
{
return -1;
}
if(num<1 || num>99)
{
return -1;
}
numbers[count]=num;
count++;
ptr = endptr;
}
if(count == SET_SIZE)
{
return 0;
}
else
{
return -1;
}
}

int are_unique(int numbers[])
{
for(int i=0; i<SET_SIZE; i++)
{
for(int j=i+1; j<SET_SIZE; j++)
{
if(numbers[i]==numbers[j])
{
return -1;
}
}
}
return 0;
}

int main(int argc, char *argv[])
{
if(argc !=5)
{
fprintf(stderr, "Error: Missing input values.\n");
return 1;
}
char *input_numbers=NULL;
char *input_file=NULL;

for(int i=0;i<argc;i++)
{
if(strcmp(argv[i],"-n")==0 && i+1<argc){
input_numbers=argv[++i];
}
else if(strcmp(argv[i],"-f")==0 && i+1<argc){
input_file=argv[++i];
}
}

if(input_numbers == NULL || input_file == NULL)
{
fprintf(stderr,"Error: Input numbers (-n) or the the input file (-f) is missing\n");
return 1;
}

int input[SET_SIZE];

if(check_input_numbers(input_numbers,input)!=0)
{
fprintf(stderr,"Error: Input numbers should have total 5 comma separated digits or numbers should be between 1-99.\n");
return 1;
}

if(are_unique(input)!=0)
{
fprintf(stderr,"Error: Unique values should be present in the input numbers\n");
return 1;
}

FILE *given_file = fopen(input_file,"r");
if(!given_file)
{
perror("Issue in opening the input file\n");
return 1;
}

char one_line_of_file[MAX_LINE_LEN];
int one_line_numbers[SET_SIZE];
int line_number = 0;
int no_of_matches[SET_SIZE + 1][MAX_LINES];
int matches_lines_count[SET_SIZE + 1] = {0};

while(fgets(one_line_of_file,sizeof(one_line_of_file),given_file) && line_number<MAX_LINES)
{
if(check_line_numbers(one_line_of_file,one_line_numbers)!=0)
{
fprintf(stderr,"Skipping the invalid line - %d in the given file\n",line_number+1);
line_number++;
continue;
}

if(are_unique(one_line_numbers)!=0)
{
fprintf(stderr,"Skipping the invalid line : %d as duplicates are present\n",line_number+1);
line_number++;
continue;
}

int count=0;
for(int i=0;i<SET_SIZE;i++)
{
for(int j=0;j<SET_SIZE;j++)
{
if(input[i]==one_line_numbers[j])
{
count++;
break;
}
}
}

if(count>0)
{
no_of_matches[count][matches_lines_count[count]]=line_number;
matches_lines_count[count]++;
}
line_number++;
}
fclose(given_file);


for(int i=SET_SIZE; i>0; i--)
{ 
if(matches_lines_count[i]>0)
{
printf("%d (",i);
for(int j=0;j<matches_lines_count[i];j++)
{
printf("%d",no_of_matches[i][j]);
if(j<matches_lines_count[i]-1)
{
printf(",");
}
}
printf(")\n");
}
}
return 0;
}





