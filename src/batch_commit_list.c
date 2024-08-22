#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#define INITIAL_CAPACITY 10000 // Initial capacity for the dynamic array
#define BATCH_SIZE 100

int isIgnored(const char* path) {
    char command[2048];
    snprintf(command, sizeof(command), "git check-ignore -q \"%s\"", path);
    int result = system(command);
    return result == 0; // Returns 0 if the file is ignored
}

void listFiles(const char* path, char*** paths, int* pathCount, int* capacity) {
    struct dirent* entry;
    DIR* dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp))) {
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

        struct stat pathStat;
        if (stat(filePath, &pathStat) != 0) {
            perror("stat");
            continue;
        }

        if (S_ISREG(pathStat.st_mode)) { // Regular file
            if (!isIgnored(filePath)) {
                if (*pathCount >= *capacity) {
                    *capacity *= 2;
                    *paths = realloc(*paths, *capacity * sizeof(char*));
                    if (*paths == NULL) {
                        perror("realloc");
                        exit(1);
                    }
                }
                (*paths)[*pathCount] = strdup(filePath);
                (*pathCount)++;
            }
        }
        else if (S_ISDIR(pathStat.st_mode)) { // Directory
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            listFiles(filePath, paths, pathCount, capacity);
        }
    }

    closedir(dp);
}

void processBatch(char** paths, int pathCount, int batchSize) {
    char command[2048]; // Declare the command variable here
    int count = 0;
    int batchNumber = 1;

    for (int i = 0; i < pathCount; i++) {
        // Add the file to git
        snprintf(command, sizeof(command), "git add \"%s\"", paths[i]);
        system(command);

        count++;

        if (count == batchSize) {
            // Commit and push the batch
            snprintf(command, sizeof(command), "git commit -m \"commit %d\"", batchNumber);
            system(command);
            system("git push");

            // Reset the count and increment the batch number
            count = 0;
            batchNumber++;
        }
    }

    // Commit and push any remaining files
    if (count > 0) {
        snprintf(command, sizeof(command), "git commit -m \"commit %d\"", batchNumber);
        system(command);
        system("git push");
    }
}

int main() {
    char** paths = malloc(INITIAL_CAPACITY * sizeof(char*));
    if (paths == NULL) {
        perror("malloc");
        return 1;
    }
    int pathCount = 0;
    int capacity = INITIAL_CAPACITY;

    listFiles(".", &paths, &pathCount, &capacity); // Start from the current directory

    printf("Total files to process: %d\n", pathCount); // Debug print

    processBatch(paths, pathCount, BATCH_SIZE); // Process in batches of 100

    // Free allocated memory
    for (int i = 0; i < pathCount; i++) {
        free(paths[i]);
    }
    free(paths);

    return 0;
}
