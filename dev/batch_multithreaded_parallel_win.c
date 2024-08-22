#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <tchar.h>

#define TABLE_SIZE 100
#define NUM_THREADS 24
#define BATCH_SIZE 100

typedef struct Node {
    char* path;
    struct Node* next;
} Node;

typedef struct {
    Node* buckets[TABLE_SIZE];
    HANDLE locks[TABLE_SIZE];
} HashTable;

unsigned int hash(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % TABLE_SIZE;
}

void insert(HashTable* table, const char* path) {
    unsigned int index = hash(path);
    WaitForSingleObject(table->locks[index], INFINITE);

    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        ReleaseMutex(table->locks[index]);
        return;
    }
    newNode->path = _strdup(path);
    if (!newNode->path) {
        perror("_strdup");
        free(newNode);
        ReleaseMutex(table->locks[index]);
        return;
    }
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;

    ReleaseMutex(table->locks[index]);
}

DWORD WINAPI addFiles(LPVOID arg) {
    HashTable* table = (HashTable*)arg;
    char command[2048];
    int count = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        WaitForSingleObject(table->locks[i], INFINITE);
        Node* current = table->buckets[i];
        while (current) {
            snprintf(command, sizeof(command), "git add \"%s\"", current->path);
            if (system(command) == -1) {
                perror("system");
            }
            count++;
            if (count % BATCH_SIZE == 0) {
                snprintf(command, sizeof(command), "git commit -m \"Batch commit %d\"", count / BATCH_SIZE);
                if (system(command) == -1) {
                    perror("system");
                }
                if (system("git push") == -1) {
                    perror("system");
                }
            }
            current = current->next;
        }
        ReleaseMutex(table->locks[i]);
    }

    // Final commit and accounting for remaining files
    if (count % BATCH_SIZE != 0) {
        snprintf(command, sizeof(command), "git commit -m \"Final batch commit\"");
        if (system(command) == -1) {
            perror("system");
        }
        if (system("git push") == -1) {
            perror("system");
        }
    }

    return 0;
}

void populateHashTable(HashTable* table) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(_T("*"), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("FindFirstFile");
        return;
    }

    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            insert(table, findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

int main() {
    HashTable table;
    memset(&table, 0, sizeof(HashTable));
    for (int i = 0; i < TABLE_SIZE; i++) {
        table.locks[i] = CreateMutex(NULL, FALSE, NULL);
        if (table.locks[i] == NULL) {
            perror("CreateMutex");
            return EXIT_FAILURE;
        }
    }

    // fill the hash table
    populateHashTable(&table);

    HANDLE threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i] = CreateThread(NULL, 0, addFiles, &table, 0, NULL);
        if (threads[i] == NULL) {
            perror("CreateThread");
            return EXIT_FAILURE;
        }
    }

    WaitForMultipleObjects(NUM_THREADS, threads, TRUE, INFINITE);

    for (int i = 0; i < NUM_THREADS; i++) {
        CloseHandle(threads[i]);
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        CloseHandle(table.locks[i]);
    }

    return 0;
}
