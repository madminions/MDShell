/*****************************************************************************************************

********************************Author ---------------------- Madhudeep Petwal---201506585
********************************Project Name----------------- MDShell
********************************Functionality-----------------Built-Ins cd,pwd,export

******************************************************************************************************
COmpile with                    g++ shellmain.cpp -Wno-write-strings

*/
#include <bits/stdc++.h>
#include <iostream>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#define MAXLIM 10000
int EXITSTATUS =1;
int MAXSIZE=1000;
char delim_pipe[2]="|";

int shellpipe(char**);
void retrievehistory(char**);
void bangcommand(char*);
void bangnumber(int);
void bangstring(char*);
int linenumber();
using namespace std;

/* the signal handling function */

void sig_handler(int signo)
{
  if (signo == SIGINT)
  {
  	//cout<<endl;
  }
    //printf("received SIGINT\n");
}
int execute_command(char **command)
{
	pid_t pid;
	int status;
	
	pid=fork();
	if(pid==0)
	{/*Child Process*/

		if (execvp(command[0], command) == -1) {
		perror("command not found");
    	}
    	exit(0);
    }
    else{
    	/*Parent Process*/
    	wait(&status);
    }
}

/*-----------------------------------------------Parser.h-----------------------------------------------------------*/

char** tokenizer(char *input,char *delim)										/*Tokenizer function returns command deparated by | */
{
	int k=0,i;
	int size=MAXSIZE;
    char *temp=input;	/*temp array Getting from strsep*/
    char **token;
   	token=(char**)malloc(sizeof(char*)*MAXSIZE);
	token[0]=(char*)malloc(sizeof(int)*20*20);
	for(i=0;i<10;i++)token[i]=token[0]+20*i;
	
    
    while (temp != NULL) {

    	token[k++]= strsep(&temp,delim);
		if(strcmp(token[k-1],"")==0)	k--;	/*Escape Empty string between */
    }
    token[k]=NULL;
    return token;
}

char** commandargs(char *input1)												/*Command splits single command to command & arguments*/
{
	int i=0,k=0,l,first,flag=0;
	int indoublequote=0,insinglequote=0;
	char *input=strdup(input1);
	char **args=NULL;
	
	args=(char**)malloc(sizeof(char*)*MAXSIZE);
	args[0]=(char*)malloc(sizeof(int)*20*20);
	for(i=0;i<10;i++)args[i]=args[0]+20*i;
	i=0;
	
	while(input[i]==' ' && input[i]!='\0')i++;						/*Skipping spaces */
	while(input[i]!=' ' && input[i]!='\0')args[0][k++]=input[i++];	/*Storing command name*/
	args[0][k]='\0';
	first=1;k=0;i++;
	for(;input[i]!='\0';i++)
	{

		while(indoublequote && input[i]!='"' && input[i]!='\0')
			{
				args[first][k++]=input[i++];
				cout<<endl;
				flag=1;

			}
			if(flag)
			{
				args[first][k]='\0';
				first++;k=0;flag=0;indoublequote=0;
				if(input[i]=='\0')break;
				continue;
			}
		while(input[i]==' ' && !indoublequote){i++;}

		if(input[i]=='"' && !indoublequote){ indoublequote=1;}

		else{
			while(input[i]!=' ' && input[i]!='\0')
				{
				args[first][k++]=input[i++];
				flag=1;
			}
			if(flag)
			{
				args[first][k]='\0';
				first++;k=0;flag=0;
			}
		}
	}
	args[first]=NULL;
return args;
}
/*=================================================Parser.h=======================================================*/

/*-----------------------------------------------Pipe.h-----------------------------------------------------------*/
void checkcommandshellpipe(char **arg)
{
	int i;
	for(i=0;arg[i]!=NULL;i++);
	
	if(i>1)shellpipe(arg);
	else
	{
		char *singlecommand=strdup(arg[0]);
		char **token=commandargs(singlecommand);
		if (strcmp(token[0],"cd")==0)							/*------------CD-------------------------------*/
		{
			size_t sizepath=1000;
			char *home=strdup(getenv("HOME"));
			char *cwd=(char*)malloc(1000*sizeof(char));
			getcwd(cwd,sizepath);
			if(strcmp(token[1],"~")==0)
				{
					chdir(home);
					setenv("PWD",home,1);
					setenv("OLDPWD",cwd,1);
				}

			else if(chdir(token[1])==-1)
			{
				perror("cannot change Directory: Not sufficient permission or dir dont exist");
			}else
			{
				char *newcwd=(char*)malloc(1000*sizeof(char));;
				getcwd(newcwd,sizepath);
				setenv("PWD",newcwd,1);
				setenv("OLDPWD",cwd,1);				
			}
		}
		else if(strcmp(token[0],"pwd")==0)							/*---------------PWD-------------------------------*/
		{
			char *pwd=strdup(getenv("PWD"));
			cout<<pwd<<endl;
		}
		else if(strcmp(token[0],"export")==0)						/*---------------EXPORT-------------------------------*/
		{
			i=0;
			int index=0;
			char var[100],value[100];
			while(token[1][i]!='=')
			{
				var[index++]=token[1][i++];
			}
			var[index]='\0';index=0;i++;
			while(token[1][i]!='\0' )
			{
				if(token[1][i]=='"')i++;
				value[index++]=token[1][i++];
				if(token[1][i]=='\0')break;
			}
			value[index]='\0';
			setenv(var,value,1);

		}
		else if(strcmp(token[0],"history")==0)						/*---------------HISTORY-------------------------------*/
		{
			retrievehistory(token);
		}
		else if(strcmp(token[0],"echo")==0)							/*---------------EChO-------------------------------*/
		{
			int i;
			char **arg=commandargs(singlecommand);
			char *echoarg;
			char argtemp[10000];
			strcpy(argtemp,arg[1]);
			i=0;
			while(arg[1][i]==' ' && arg[1][i]!='\0')arg[1]++;

			if(arg[1][0]=='$')
				{
					i=0;
					while(arg[1][i]!=' ' && arg[1][i]!='\0')i++;
					arg[1][i]='\0';
					echoarg=getenv(arg[1]+1);
					cout<<echoarg<<endl;
				}
			else
			{
				cout<<argtemp<<endl;
			}

		}
		else if(singlecommand[0]=='!')								/*---------------BANG COMMAND----------------------------*/
		{
			char no[10000];
			int i=1,k=0;
			if((singlecommand[1]<='9' && singlecommand[1]>='0') || (singlecommand[1]=='-' && singlecommand[2]>='0' && singlecommand[2]<='9'))
			{
				while(singlecommand[i])
					{
						no[k++]=singlecommand[i++];
					}
				no[k]='\0';
				int number=atoi(no);
				bangnumber(number);
			}
			else
			{
				if(singlecommand[1]=='!')
				{
					int length=linenumber();
					bangnumber(length-1);
				}
				bangstring(singlecommand+1);
			}
			
		}
		else{															//not built in commnad so execute using execvp
				int status;
				if(!fork())//child
				{
					if(execvp(token[0],token)==-1)
						cout<<"Invalid Command"<<endl;
					exit(0);
				}
				else{
					wait(&status);
				}
			}
	}//noofcommands > 1 else

}//function ends

int shellpipe(char **arg)
{
   int p[2];
   pid_t cpid;
   int fd = 0;
   while (*arg != NULL)
   {
      if(pipe(p)==-1)
      {
      	perror("Pipe cannot be created");
      }
      if ((cpid = fork()) == -1)
         exit(EXIT_FAILURE);
      else if (cpid == 0)      /*CHild*/
      {
         dup2(fd, 0); //old ---> input
         if (*(arg+1) != NULL)
            dup2(p[1], 1);
         close(p[0]);
         char **q=commandargs(arg[0]);
         if(execvp(q[0],q)==-1)
         {
         	perror("Command Not found");
         }
         exit(EXIT_FAILURE);
      }
      else						/*Parent*/
      {
         wait(NULL);
         close(p[1]);
         fd = p[0]; //input --- > next command
         arg++;
      }
   }

}

/*=================================================Pipe.h=======================================================*/

/*-----------------------------------------------history.h---------------------------------------------------------*/

void savehistory(char *argv)
{
	int i=0;
	ofstream outputfile;
	char *homepath=strdup(getenv("HOME"));
	strcat(homepath,"/history.txt");
	outputfile.open(homepath,ios::out|std::ios_base::app);
	if(!outputfile.is_open())
		perror("File not opened");
	else
	{
		outputfile<<argv<<endl;
	}
	
	outputfile.close();
}
void retrievehistory(char **token)
{
	int i=1;
	ifstream inputfile;
	string k;
	char *homepath=strdup(getenv("HOME"));
	strcat(homepath,"/history.txt");
	inputfile.open(homepath,ios::in);

	if (inputfile.is_open())
	{
		while(getline(inputfile,k))
		{
			cout<<i<<"	"<<k<<endl;
			i++;
		}
	}
	else {perror("File Not opened");}
	inputfile.close();

}
void bangnumber(int n)										/************command !<Number> matches most recent command******/
{
	int i=1;
	ifstream inputfile;
	string k;
	char **p;

	p=(char**)malloc(sizeof(char*)*MAXSIZE);
	p[0]=(char*)malloc(sizeof(int)*20*20);
	for(i=0;i<10;i++)p[i]=p[0]+20*i;

	i=1;
	int lineno=linenumber();
	char *homepath=strdup(getenv("HOME"));
	strcat(homepath,"/history.txt");
	inputfile.open(homepath,ios::in);

	if (inputfile.is_open())
	{
		while(getline(inputfile,k))
		{
			if(n>0 && i==n)break;
			if(n<0 && i==n+lineno-1)break;
			i++;
		}
		cout<<k<<endl;
		i=0;
		while(k[i]!='\0'){
			p[0][i]=k[i];i++;}
			p[0][i]='\0';
		p[1]=NULL;
		char **token=tokenizer(p[0],delim_pipe);
		checkcommandshellpipe(token);
	}
	else {perror("File Not opened");}
	inputfile.close();
}
void bangstring(char *bangcommand)						/************command !<string> matches most recent command************/
{
		int i=1,n;
	ifstream inputfile;
	string k,command;
	char **p;

	p=(char**)malloc(sizeof(char*)*MAXSIZE);
	p[0]=(char*)malloc(sizeof(int)*20*20);
	for(i=0;i<10;i++)p[i]=p[0]+20*i;

	i=1;
	char *homepath=strdup(getenv("HOME"));
	strcat(homepath,"/history.txt");
	inputfile.open(homepath,ios::in);

	if (inputfile.is_open())
	{
		while(getline(inputfile,k))
		{
			if(k[0]==bangcommand[0])command=k;
		}
		cout<<command<<endl;
		i=0;
		while(command[i]!='\0'){
			p[0][i]=command[i];i++;}
		p[0][i]='\0';
		p[1]=NULL;
		char **token=tokenizer(p[0],delim_pipe);
		checkcommandshellpipe(token);
	}
	else {perror("File Not opened");}
	inputfile.close();

}

int linenumber()
{
	int i=1;
	ifstream inputfile;
	string k;
	char *homepath=strdup(getenv("HOME"));
	strcat(homepath,"/history.txt");
	inputfile.open(homepath,ios::in);

	if (inputfile.is_open())
	{
		while(getline(inputfile,k))
		{
			i++;
		}
	}
	else {perror("File Not opened");}
	inputfile.close();
	return i;

}


/*=================================================history.h=======================================================*/


/*-----------------------------------------------MAIN()---------------------------------------------------------*/
int main()
{
	int i,j;
	char **token,**execute;
	char input[MAXLIM];
	char exit_string[10]="exit";
	char delim_pipe[2]="|";
	char delim_space[2]=" ";
	while(1)
	{
		
		cout<<"MDShell>";

		if(fgets(input,sizeof(input),stdin));						/*Get input from the user*/

		int length=strlen(input);
		input[length-1]='\0';	
		
		if(strcmp(input,exit_string)==0)
		{
			cout<<"See You Soon"<<endl;
			break;
		}
		
		if(strcmp(input,"")){
			input[length-1]='\0';										/*Removing \n which was added by fgets function*/
			if(input[0]!='!')
			savehistory(input);
			token=tokenizer(input,delim_pipe);
			checkcommandshellpipe(token);
		}
		if (signal(SIGINT, sig_handler) == SIG_ERR);

//		if(a!="\n")
}
	

	return 0;

}
/*=================================================MAIN()=======================================================*/