#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAIN_FILE "scores.csv"
#define TEMP_SCORES_FILE "tempScores.csv"
#define TEMP_LEADERBOARDS_FILE "tempLeaderboards.csv"

bool isUsernameTaken(char* usernameToSearch) {
	FILE *filePointer;
	
	filePointer = fopen(MAIN_FILE, "r");
	
	// If file doesnt exist, create it.
	if(!filePointer) {
		// Create the file
		filePointer = fopen(MAIN_FILE, "w");
		fclose(filePointer);
		
		return false;
	}
	
	// Search entire file until the end
    char line[1024];
    
	while (fgets(line, 1024, filePointer)) {
		// If it is found, then return true
	    if(strstr(line, usernameToSearch) != 0) {
	    	fclose(filePointer);
			return true;
	    }
    }
	
	fclose(filePointer);
	return false;
}

void saveUserData(char* username, int winnings) {
	FILE *filePointer;
	
	filePointer = fopen(MAIN_FILE, "a");
	fprintf(filePointer, "%s,%d\n", username, winnings);
	fclose(filePointer);
}


char* getField(char* line, int position) {
    char* token;
    token = strtok(line, ",");
    
    while(token) {
    	if(!--position)
    		return token;
    	
    	token = strtok(NULL, ",");
	}
	
    return NULL;
}

void generateLeaderboards() {
	// Sort CSV file using selection sort.
	// The goal here is to get the top 10
	// Usernames with most scores/winnings
	
	int counter = 0, linesToSkip[10];
	FILE *filePointer, *tempFilePtr;
	
	tempFilePtr = fopen(TEMP_LEADERBOARDS_FILE, "w"); // Create a temporary file
	
	// Read entire file depending on num of lines (10 is max)
	// Until we get a sorted leaderboards from our CSV file
    while(counter < 10) {
    	int lineNumber = 0, highestLineNumber;
		int highestScore = 0;
		char *highestUsername, line[1024];
		
	   	filePointer = fopen(MAIN_FILE, "r"); // Open scores file to be read.

		// Check if the game hasnt been initialized
		if(filePointer == NULL) {
			// Create the file, then return
			filePointer = fopen(MAIN_FILE, "w");
			fclose(filePointer);
			break;
		}
		
		while (fgets(line, 1024, filePointer)) {
			// Check if current line is already sorted
			// If it is then skip this iteration, and
			// Continue to the next one
			bool shouldSkipLine = false;
			for(int i = 0; i < counter; i++) {
				if(linesToSkip[i] == lineNumber)
					shouldSkipLine = true;	
			}
			
			if(shouldSkipLine) {
				lineNumber++;
				continue;
			}
			
		    char* tmp = strdup(line);
	        char* username = getField(tmp, 1);
	        
		    tmp = strdup(line);
	        int score = atoi(getField(tmp, 2));
	        
	        if(score > highestScore) {
	        	highestUsername = username; // Set the new highest line
	        	highestLineNumber = lineNumber; // Set the new highest line number
	        	highestScore = score; // Set the new highest score
			}
			
	        free(tmp); // getField clogs memory.
	        lineNumber++;
	    }
	    
	    // If file is empty, then don't sort anymore
	    if(lineNumber == 0)
	    	break;
	    
	    linesToSkip[counter++] = highestLineNumber;
	    fprintf(tempFilePtr, "> Rank #%d:\t\t%s\t\t%d\n", counter, highestUsername, highestScore);
	    
	    if(counter == lineNumber)
	    	break;
	}
	
	fclose(filePointer);
	fclose(tempFilePtr);
	
	return;
}

void printFile(char* fileName) {
	FILE *filePointer;
	
	filePointer = fopen(fileName, "r");
	
	if(!filePointer) {
		perror(fileName);
		return;
	}
	
	// Print each lines
    char line[1024];
	while (fgets(line, 1024, filePointer)) {
	    printf("%s", line);
    }
    
    fclose(filePointer);
}
