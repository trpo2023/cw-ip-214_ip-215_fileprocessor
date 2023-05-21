#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
//#include <conio.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_FILELENGTH 500

int isFile(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void scandir(char *dirname, char buffer[][MAX_FILELENGTH], int *bufferIndex, int needFile)
{
	DIR *dir;
	struct dirent *ent;
	
	
	if ((dir = opendir(dirname)) == NULL)
	{
		perror("Error opening");
		exit(1);
	}
	
	size_t dirLen = strlen(dirname);
	char pathTmp[MAX_FILELENGTH];
	strcpy(pathTmp, dirname);
	
	while ((ent = readdir(dir)) != NULL)
	{
		strcpy(pathTmp+dirLen, ent->d_name);
		if(ent->d_name[0]!='.' && isFile(pathTmp) == needFile) {
			strcpy(buffer[(*bufferIndex)++], pathTmp);
		}
	}
	if (closedir(dir) != 0)
	{
		perror("Error closing");
	}
	
}

void createFile(char *dst) {
	FILE *file;
	if((file = fopen(dst, "w"))==NULL) {
		perror("Error while creating file");
		exit(1);
	}
	fclose(file);
}

void removeFiles(char *dir) {
	char buffer[20][MAX_FILELENGTH];
	int bufferIndex = 0;
	scandir(dir, buffer, &bufferIndex, 1);
	for(int i=0; i<bufferIndex; ++i) {
		if(remove(buffer[i])) {
			perror("Error while removing");
			exit(1);
		}
	}
}

int main() {
	setlocale(0, "");
	char buffer[20][MAX_FILELENGTH];
	int bufferIndex = 0;
	scandir("./tests/", buffer, &bufferIndex, 0);
	
	char filebuffer[30][MAX_FILELENGTH];
	int fileBufferIndex = 0;
	
	char resultBuffer[30][MAX_FILELENGTH];
	int resultBufferIndex = 0;
	
	char patternPath[MAX_FILELENGTH];
	char inputPath[MAX_FILELENGTH];
	char assertPath[MAX_FILELENGTH];
	
	char executionStr[200];
	
	char tmpDir[100];
	for(int i=0; i<bufferIndex; ++i) {
		removeFiles("./tmpTests/");
		strcpy(inputPath, buffer[i]);
		strcat(inputPath, "/input/");
		
		strcpy(patternPath, buffer[i]);
		strcat(patternPath, "/pattern.txt");
		
		strcpy(assertPath, buffer[i]);
		strcat(assertPath, "/assert/");
		
		fileBufferIndex = 0;
		resultBufferIndex = 0;
		printf("%s:\n", inputPath);
		
		scandir(assertPath, resultBuffer, &resultBufferIndex, 1);
		
		scandir(inputPath, filebuffer, &fileBufferIndex, 1);
		for(int j=0; j<fileBufferIndex; ++j) {
			//printf("%s\n", filebuffer[j]);
			strcpy(tmpDir, "./tmpTests/");
			strcpy(tmpDir+11, strrchr(filebuffer[j], '/')+1);
			printf("%s\n", tmpDir);
			createFile(tmpDir);
		}
		sprintf(executionStr, "vanya.exe -d ./tmpTests/ -p %s\n", patternPath);
		printf("%s", executionStr);
		if(system(executionStr)!=0) {
			perror("Exit code not 0\n");
			exit(1);
		}
		
		fileBufferIndex=0;
		scandir("./tmpTests/", filebuffer, &fileBufferIndex, 1);
		if(fileBufferIndex != resultBufferIndex) {
			printf("%s failed: files not match\n", buffer[i]);
			exit(1);
		}
		
		for(int j=0; j<fileBufferIndex; ++j) {
			strcpy(filebuffer[j], strrchr(filebuffer[j], '/')+1);
			strcpy(resultBuffer[j], strrchr(resultBuffer[j], '/')+1);
		}
		
		for(int j=0; j<fileBufferIndex; ++j) {
			if(strcmp(filebuffer[j], resultBuffer[j])!=0) {
				printf("%s failed: expected %s, getted %s", buffer[i], resultBuffer[j], filebuffer[j]);
				exit(1);
			}
		}
		
		
		
		printf("\n");
	}
	
	printf("All tests passed\n");
	return 0;
}
