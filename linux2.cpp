//Steven Nguyen, Brandon No, Mason Kam
//linux2.cpp
//Purpose: To be able to create a working shell that runs one commands,
//  or a pipeline of commands that prints the exit status for each process
//  after the command is terminated.
//Input: Command(s): Either single, or multiple that will be pipelined
//Process: Parsing, forking child, executing commands, and pipling commands.
//Output: Result of command(s) given. 

#include<iostream>
#include<string>
#include<vector>
#include<sys/wait.h>
using namespace std;

struct ComdList
{
  char ** args;
  ComdList *next;
};

struct BuiltinCmd{
  string name;
  int (*func)(char**);
};

int builtinCD(char **args);
//Handles cd command

int builtinexit(char **args);
//Handles exit command

int parseString(string _mainCommand, ComdList *&_head);
//Pre: none
//Post: subCommand is maybe empty
//return number of commands

void commandParse(vector<string> const &_subCommand,
				  ComdList *&_head, int numOfComds);
//Parses one command line at a time

void addComd(ComdList *&_head, vector<string> const &_args);
//Insert command in linked list

void externalFunc(ComdList *&_head, int _numComds);
//Handles execution of commands

void pipeline(ComdList *&_head, int _numComds); 
//Handles piplining commands

void del(ComdList *&_head);
//Deallocates one node

void runSource(int pfd[], char **args);
//Handles first pipe

void runDest(int pfd[], char **args);
//Handles last pipe

void runPipe(int in[], int out[], char** args);
//Handles all intermediate pipes

void runFunc(ComdList *&_head, int _numComds);
//Decides whether command is built in or not

void updateDir(char *_dir);
//updating the directory of the process

int isBuiltin(BuiltinCmd _table[], ComdList *&_head);
//Check if command is built in and returns -1 if error, or 0 or 1 depending
// on built-in command.

const int MAXARGS = 50,
  MAXTOKEN = 100,
  TABLESIZE = 2,
  CLEAR = 50;

string dir = "";
BuiltinCmd cmdTable[TABLESIZE];


int main()
{
  string mainCommand;
  ComdList *ComdListHead = 0;
  int numComds;
  char *temp = getcwd(NULL, 0);

  for(int a = 0; a < CLEAR; a++)
	cout << endl;
  
  for(int i = 0; i < TABLESIZE; i++)
	switch (i){
	case 0:
	  cmdTable[i].name = "cd";
	  cmdTable[i].func = &builtinCD;
	  break;
	case 1:
	  cmdTable[i].name = "exit";
	  cmdTable[i].func = &builtinexit;
	  break;
	}

  cout << endl << endl << endl;

  updateDir(temp);
  delete temp;
  cout << endl << "[Test~" << dir << "]$ ";
  getline(cin, mainCommand);

  while(true){
	numComds = parseString(mainCommand, ComdListHead);
	if(ComdListHead != 0){
	  runFunc(ComdListHead, numComds);
	  del(ComdListHead);
	}
	cout << endl << "[Test~" << dir << "]$ ";
	getline(cin, mainCommand);	
  }

  cout << endl << endl << endl;
  return 0;
}


int parseString(string _mainCommand, ComdList *&_head)
{
  vector<string> subCommand;
  string temp;
  int count = 0;
  
  //Get each command and put it in to the string vector
  for(unsigned i = 0; i < _mainCommand.length(); i++)
	//Dealing with " "
	if(_mainCommand[i] == '\"'){
	  if(temp == "")
		count++;
	  temp += _mainCommand[i];
	  i++;
	  while(_mainCommand[i] != '\"'){
		temp += _mainCommand[i];
		i++;
	  }
	  temp += _mainCommand[i];

	//Dealing with ' '
	}else if(_mainCommand[i] == '\''){
	  if(temp == "")
		count++;
	  temp += _mainCommand[i];
	  i++;
	  while(_mainCommand[i] != '\''){
		temp += _mainCommand[i];
		i++;
	  }
	  temp += _mainCommand[i];

	  //Dealing with |
	}else if(_mainCommand[i] == '|'){
	  subCommand.push_back(temp);
	  temp = "";
	  
	}else{
	  if(temp == "")
		count++;
	  temp += _mainCommand[i];
	}

  subCommand.push_back(temp);
  commandParse(subCommand, _head, count);  

  return count;
}


void commandParse(vector<string> const&_subCommand,
				  ComdList *&_head, int numOfComds)
{
  vector<string> token;  
  string tempCommand, tempToken = "";
  
  for(int i = 0; i < numOfComds; i++){
	tempCommand = _subCommand[i];
	
	for(unsigned j = 0; j < tempCommand.length(); j++){
	  //Dealing with " "
	  if(tempCommand[j] == '\"'){
		if(tempToken != "")
		  token.push_back(tempToken);
		
		tempToken = "";
		j++;
		
		while(tempCommand[j] != '\"'){
		  tempToken += tempCommand[j];
		  j++;
		}
		
		token.push_back(tempToken);
		tempToken = "";

		//Dealing with ' '
	  }else if(tempCommand[j] == '\''){
		if(tempToken != "")
		  token.push_back(tempToken);
		
		tempToken = "";
		j++;
		
		while(tempCommand[j] != '\''){
		  tempToken += tempCommand[j];
		  j++;
		}
		
		token.push_back(tempToken);
		tempToken = "";

		//Dealing with space
	  }else if (tempCommand[j] == ' '){
		if(tempToken != ""){
		  token.push_back(tempToken);
	 	  tempToken = "";
		}
		
	  }else{
		tempToken += tempCommand[j];
	  }
	}
	
	if(tempToken != "")
	  token.push_back(tempToken);
	
	tempToken = "";
	addComd(_head, token);
	token.clear();
  }
}


void addComd(ComdList *&_head, vector<string> const &_args)
{  
  char** tempArgs = new char*[_args.size() + 1];
  ComdList *temptr;
  
  for(unsigned i = 0; i < _args.size(); i++){
	tempArgs[i] = new char[MAXTOKEN];
	
	for(unsigned j = 0; j < _args[i].length(); j++)
	  tempArgs[i][j] = _args[i][j];
  }
  
  tempArgs[_args.size()] = 0;
  
  if(_head == 0){
	_head = new ComdList;
	_head->next = 0;
	_head->args = tempArgs;
	
  }else{
	temptr = _head;
	
	while(temptr->next != 0)
	  temptr = temptr->next;
	
	temptr->next = new ComdList;
	temptr = temptr->next;
	temptr->next = 0;
	temptr->args = tempArgs;
  }
}


void externalFunc(ComdList *&_head, int _numComds)
{
  int pid, status;
  
  if(_numComds == 1){
	switch(pid = fork()){
	case 0:
	  execvp(_head->args[0], _head->args);
	  perror(_head->args[0]);
	default:
	  while((pid = wait(&status)) != -1)
		cout << "Process " << pid << " exit with "
			 << WEXITSTATUS(status) << endl;
	  break;
	case -1:
	  perror("fork");
	  exit(1);
	}
  }else
	pipeline(_head, _numComds);
}


void pipeline(ComdList *&_head, int _numComds)
{
  int pid, status;
  int **fd = new int*[_numComds - 1];
  ComdList *temp = _head;
  
  for(int i = 0; i < _numComds - 1; i++)
	fd[i] = new int[2];
  for(int i = 0; i < _numComds - 1; i++)
	pipe(fd[i]);
  
  runSource(fd[0], temp->args);

  for(int i = 0; i < _numComds-2; i++){
	temp = temp->next;
	runPipe(fd[i+1], fd[i], temp->args);
	close(fd[i][0]);
	close(fd[i][1]);
  }
  
  temp = temp->next;
  runDest(fd[_numComds-2], temp->args);

  for(int i = 0; i < _numComds - 1; i++){
	close(fd[i][0]);
	close(fd[i][1]);
  }
  
  while((pid = wait(&status)) != -1)
	cout << "Process " << pid << " exit with "
		 << WEXITSTATUS(status) << endl;
}


int isBuiltin(BuiltinCmd _table[], ComdList *&_head)
{
  string temp(_head->args[0]);
  
  for(int i = 0; i < TABLESIZE; i++)
	if(temp == _table[i].name){
	  return i;
	}
  return -1;
}


int builtinCD(char **args)
{
  int count = 0, check;
  char *temp;
  
  while(args[count] != 0)
	count++;
  
  if(count > 2)
	return -1;
  else{
	if(args[1] == 0){
	  check = chdir(getenv("HOME"));
	  temp = getcwd(NULL, 0);
	}
	else{
	  check = chdir(args[1]);
	  temp = getcwd(NULL, 0);
	}

	if(temp != 0)
	  for(unsigned i = 0; i < strlen(temp); i++){
		cout << temp[i];
	  }
	cout << endl;
  
	if(check == -1)
	  cout << "There is no such directory" << endl;
	else{
	  updateDir(temp);
	}
  }
  
  delete temp;
  
  return check;
}


void updateDir(char *_dir)
{
  string tempDir = "";
  int index;

  for(unsigned i = 0; i < strlen(_dir); i++){
	if(_dir[i] == '/'){
	  index = i;
	}
  }
  
  index++;

  for(unsigned i = index; i <strlen(_dir); i++)
	tempDir += _dir[i];

  dir = tempDir;
}


int builtinexit(char **args)
{
  if(args[1] == 0)
	exit(0);
  else
	exit(atoi(args[1]));
}


void del(ComdList *&_head)
{
  ComdList *delptr = _head;
  
  while(_head != 0){
	delptr = _head;
	_head = _head->next;
	delete delptr;
  }
}

void runSource(int pfd[], char ** args)
{
  int pid;
  
  switch(pid = fork()){
  case 0:
	dup2(pfd[1], 1);
	close(pfd[0]);
	execvp(args[0], args);
	perror(args[0]);
  default:
	break;
  case -1:
	perror("fork");
	exit(1);
  }
}


void runPipe(int in[], int out[], char** args)
{
  int pid;
  
  switch(pid = fork()){
  case 0:
	dup2(out[0], 0); //out from the previous pipe
	close(out[1]);
	dup2(in[1], 1); //in to the next pipe
	close(in[0]);
	execvp(args[0], args);
	perror(args[0]);
  default:
	break;
  case -1:
	perror("fork");
	exit(1);
  }  
}


void runDest(int pfd[], char **args)
{
  int pid;
  
  switch(pid = fork()){
  case 0:
	dup2(pfd[0], 0);
	close(pfd[1]);
	execvp(args[0], args);
	perror(args[0]);
  default:
	break;
  case -1:
	perror("fork");
	exit(1);
  }  
}


void runFunc(ComdList *&_head, int _numComds)
{
  int funcNum;
  
  funcNum = isBuiltin(cmdTable, _head);
  if(funcNum == -1){
	externalFunc(_head, _numComds);
  }else{
	cmdTable[funcNum].func(_head->args);
  }
}

