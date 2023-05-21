#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define MAX_FILELENGTH 500

#define CODE_TEXT 0
#define CODE_INC 1
#define CODE_NAME 2
#define CODE_EXT 3



char buffer[200][MAX_FILELENGTH];
int bufferIndex = 0;

int isFile(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void scandir(char *dirname)
{
	DIR *dir;
	struct dirent *ent;
	
	printf("Reading '%s':\n", dirname);
	
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
		if(isFile(pathTmp)) {
			strcpy(buffer[bufferIndex++], pathTmp);
		}
	}
	if (closedir(dir) != 0)
	{
		perror("Error closing");
	}
	
	printf("Files:\n");
	for(int i=0; i<bufferIndex; ++i) {
		printf("%s\n", buffer[i]);
	}
}

struct renameCode {
	int code;
	char *txt;
};

int getCode(char **pattern) {
	if(*pattern == strstr(*pattern, "<Inc>")) {
		*pattern = *pattern + 5;
		return CODE_INC;
	}
	if(*pattern == strstr(*pattern, "<Name>")) {
		*pattern = *pattern + 6;
		return CODE_NAME;
	}
	if(*pattern == strstr(*pattern, "<Ext>")) {
		*pattern = *pattern + 5;
		return CODE_EXT;
	}
	*pattern = *pattern + 1;
	return CODE_TEXT;
}

void massRename(char *pattern) {
	struct renameCode steps[20];
	char strArray[20][MAX_FILELENGTH];
	int arrayIndex = 0;
	int strArrayIndex = 0;
	
	char *pindex = 0;
	while((pindex = strchr(pattern, '<')) != NULL) {
		strncpy(strArray[strArrayIndex], pattern, pindex-pattern);
		strArray[strArrayIndex][pindex-pattern]=0;
		steps[arrayIndex].txt=strArray[strArrayIndex++];
		steps[arrayIndex].code = CODE_TEXT;
		arrayIndex++;
		pattern = pindex;
		steps[arrayIndex].code = getCode(&pattern);
		if(steps[arrayIndex].code == CODE_TEXT) {
			strcpy(strArray[strArrayIndex], "<");
			steps[arrayIndex].txt = strArray[strArrayIndex++];
		}
		arrayIndex++;
	}
	if(*pattern != 0) {
		strcpy(strArray[strArrayIndex], pattern);
		steps[arrayIndex].code = CODE_TEXT;
		steps[arrayIndex].txt = strArray[strArrayIndex++];
		arrayIndex++;
	}
	char *name;
	char *extension;
	char nname[MAX_FILELENGTH];
	char newFileName[MAX_FILELENGTH];
	for(int i=0; i<bufferIndex; ++i) {
		char *lastDelim = strrchr(buffer[i], '/');
		strcpy(nname, lastDelim+1);
		name = strtok(nname, ".");
		extension = strtok(NULL, ".");
		char *tmpStr = newFileName;
		strncpy(tmpStr, buffer[i], lastDelim-buffer[i]+1);
		tmpStr += lastDelim-buffer[i]+1;
		for(int j=0; j<arrayIndex; ++j) {
			if(steps[j].code == CODE_TEXT) {
				sprintf(tmpStr, "%s", steps[j].txt);
			}
			else if(steps[j].code == CODE_INC) {
				sprintf(tmpStr, "%d", i);
			} else if(steps[j].code == CODE_NAME) {
				sprintf(tmpStr, "%s", name);
			} else if(steps[j].code == CODE_EXT) {
				if(extension != NULL) {
					sprintf(tmpStr, "%s", extension);
				}
			}
			tmpStr = tmpStr + strlen(tmpStr);
		}
		printf("%s -> %s\n", buffer[i], newFileName);
		rename(buffer[i], newFileName);
	}
	
}

int main(int argc, char* argv[]) {
	setlocale(0, "");
	
	char *directory = "./";
	char *patternPath = 0;
	for(int i=1; i<argc; ++i) {
		if(strcmp(argv[i], "-d")==0) {
			if(i+1>=argc) {
				perror("no argument for -d");
				exit(1);
			}
			directory = argv[i+1];
			i++;
		}
		else if(strcmp(argv[i], "-p")==0) {
			if(i+1>=argc) {
				perror("no argument for -p");
				exit(1);
			}
			patternPath = argv[i+1];
			i++;
		}
	}
	
	if(directory[strlen(directory)-1]!='/') {
		perror("directory should end with '/' char");
		exit(1);
	}
	
	if(patternPath == 0) {
		perror("no pattern file selected");
		exit(1);
	}
	
	FILE *patternFile; 
	if((patternFile= fopen(patternPath, "r"))==NULL) {
		perror("Error opening patternFile");
		exit(1);
	}
	char pattern[500];
	size_t len = 0;
	if((len=fread(pattern, sizeof(char), sizeof(pattern), patternFile) )<= 0) {
		perror("Error reading patternFile");
		exit(1);
	}
	pattern[len] = 0;
	fclose(patternFile);
	printf("Pattern: %s\n", pattern);
	
	scandir(directory);
	
	printf("\nStart Renaming:\n");
	massRename(pattern);
	return 0;
}
