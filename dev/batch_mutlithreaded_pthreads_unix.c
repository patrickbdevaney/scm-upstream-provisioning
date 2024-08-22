#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <dirent.h>

#define TABLE_SIZE 100
#define NUM_THREADS 24
#define BATCH_SIZE 100

typedef struct Node {
    char *path;
    struct Node *next;
} Node;

typedef struct {
    Node *buckets[TABLE_SIZE];
    pthread_mutex_t locks[TABLE_SIZE];
} HashTable;

unsigned int hash(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % TABLE_SIZE;
}

void insert(HashTable *table, const char *path) {
    unsigned int index = hash(path);
    pthread_mutex_lock(&table->locks[index]);

    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        perror("malloc");
        pthread_mutex_unlock(&table->locks[index]);
        return;
    }
    newNode->path = strdup(path);
    if (!newNode->path) {
        perror("strdup");
        free(newNode);
        pthread_mutex_unlock(&table->locks[index]);
        return;
    }
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;

    pthread_mutex_unlock(&table->locks[index]);
}

void *addFiles(void *arg) {
    HashTable *table = (HashTable *)arg;
    char command[2048];
    int count = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        pthread_mutex_lock(&table->locks[i]);
        Node *current = table->buckets[i];
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
        pthread_mutex_unlock(&table->locks[i]);
    }

    // Final commit and push for remaining files
    if (count % BATCH_SIZE != 0) {
        snprintf(command, sizeof(command), "git commit -m \"Final batch commit\"");
        if (system(command) == -1) {
            perror("system");
        }
        if (system("git push") == -1) {
            perror("system");
        }
    }

    return NULL;
}

void populateHashTable(HashTable *table) {
    struct dirent *entry;
    DIR *dp = opendir(".");
    if (dp == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) {
            insert(table, entry->d_name);
        }
    }

    closedir(dp);
}

int main() {
    HashTable table;
    memset(&table, 0, sizeof(HashTable));
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (pthread_mutex_init(&table->locks[i], NULL) != 0) {
            perror("pthread_mutex_init");
            return EXIT_FAILURE;
        }
    }

    // filling hash table
    populateHashTable(&table);

    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, addFiles, &table) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        pthread_mutex_destroy(&table->locks[i]);
    }

    return 0;
}
