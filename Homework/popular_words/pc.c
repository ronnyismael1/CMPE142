#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// cd /mnt/c/Users/rismael/Documents/SJSU/F23/CMPE142/Homework/popular_words
// gcc -g -std=gnu2x -Wall -o pc pc.c -lpthread

// Declare a global variable and a mutex to protect it
int finished_readers = 0;
pthread_mutex_t finished_readers_lock;

typedef struct node {
    char *data;
    struct node *next;
} Node;

typedef struct queue {
    Node *head;
    Node *tail;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Queue;

typedef struct WordCount {
    char *word;
    int count;
    struct WordCount *next;
} WordCount;

typedef struct {
    WordCount *entries[256];
} HashTable;

typedef struct {
    char *filename;
    Queue *queues[4];
    int num_readers;
} ReaderArgs;

typedef struct {
    Queue *queue;
} PrinterArgs;

typedef struct {
    Queue *queue;
    HashTable *hashtable;
    int max_count;
} CounterArgs;

Queue* create_queue() {
    Queue *q = malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
}

HashTable* create_hashtable() {
    HashTable *ht = malloc(sizeof(HashTable));
    for (int i = 0; i < 256; i++) {
        ht->entries[i] = NULL;
    }
    return ht;
}

void insert_word(HashTable *ht, char *word, int count) {
    int index = hash(word) % 256;
    WordCount *entry = ht->entries[index];
    while (entry != NULL) {
        if (strcmp(entry->word, word) == 0) {
            entry->count += count;
            return;
        }
        entry = entry->next;
    }
    // If the word was not found, create a new entry
    WordCount *new_entry = malloc(sizeof(WordCount));
    new_entry->word = strdup(word);
    new_entry->count = count;
    new_entry->next = ht->entries[index];
    ht->entries[index] = new_entry;
}

int get_max_count(HashTable *ht) {
    int max_count = 0;
    for (int i = 0; i < 256; i++) {
        WordCount *entry = ht->entries[i];
        while (entry != NULL) {
            if (entry->count > max_count) {
                max_count = entry->count;
            }
            entry = entry->next;
        }
    }
    return max_count;
}

void enqueue(Queue *q, char *data) {
    Node *newNode = malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;

    pthread_mutex_lock(&q->lock);
    if (q->tail != NULL) {
        q->tail->next = newNode;
        q->tail = newNode;
    } else {
        q->head = q->tail = newNode;
    }
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}

char* dequeue(Queue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->head == NULL) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    Node *temp = q->head;
    char *data = temp->data;
    if (data == NULL) {  // Check for sentinel value
        pthread_mutex_unlock(&q->lock);
        free(temp);
        return NULL;
    }

    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->lock);

    free(temp);
    return data;
}

void *enqueue_words_from_file(void *arg) {
    ReaderArgs *args = (ReaderArgs *)arg;
    char *filename = args->filename;
    Queue **queues = args->queues;
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror(filename);
        return;
    }

    char *word = NULL;
    while (fscanf(file, "%ms", &word) == 1) {
        // Use the bottom bit of the first letter to decide the queue
        int queue_index = word[0] & 1;
        enqueue(queues[queue_index], word);
        word = NULL;  // Set word to NULL so that a new string will be allocated in the next fscanf
    }
    // After reading all words from the file
    pthread_mutex_lock(&finished_readers_lock);
    finished_readers++;
    if (finished_readers == args->num_readers) {  // Use args->num_readers instead of argc - 1
        for (int i = 0; i < 4; i++) {
            enqueue(queues[i], NULL);  // Add sentinel value
        }
    }
    pthread_mutex_unlock(&finished_readers_lock);
    fclose(file);
}

void *count_words(void *arg) {
    CounterArgs *args = (CounterArgs *)arg;
    Queue *queue = args->queue;
    HashTable *hashtable = create_hashtable();
    char *word;
    int max_count = 0;
    while ((word = dequeue(queue)) != NULL) {
        insert_word(hashtable, word, 1);
        free(word);
    }
    max_count = get_max_count(hashtable);
    args->hashtable = hashtable;
    args->max_count = max_count;
    return args;
}

int hash(const char *s) { 
   unsigned short h = 0; 
   for (;*s; s++) h += (h >> 4) + *s + (*s << 9);  
   return h; 
}

int main(int argc, char *argv[]) {
    // If no files are passed, return immediately
    if (argc < 2) {
        return 0;
    }

    // Create 4 queues
    Queue *queues[4];
    for (int i = 0; i < 4; i++) {
        queues[i] = create_queue();
    }

    // Initialize the mutex
    pthread_mutex_init(&finished_readers_lock, NULL);

    // Create and join reader threads
    pthread_t reader_threads[argc - 1];
    ReaderArgs reader_args[argc - 1];
    for (int i = 1; i < argc; i++) {
        reader_args[i - 1].filename = argv[i];
        reader_args[i - 1].num_readers = argc - 1;  // Set num_readers
        memcpy(reader_args[i - 1].queues, queues, sizeof(queues));
        pthread_create(&reader_threads[i - 1], NULL, enqueue_words_from_file, &reader_args[i - 1]);
    }
    // Ensure sentinel values are added to the queues
    for (int i = 0; i < argc - 1; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    // Create and join counter threads
    pthread_t counter_threads[4];
    CounterArgs counter_args[4];
    for (int i = 0; i < 4; i++) {
        counter_args[i].queue = queues[i];
        pthread_create(&counter_threads[i], NULL, count_words, &counter_args[i]);
    }
    CounterArgs *counter_results[4];
    for (int i = 0; i < 4; i++) {
        pthread_join(counter_threads[i], (void **)&counter_results[i]);
    }

    // Merge the results
    HashTable *merged_hashtable = create_hashtable();
    for (int i = 0; i < 4; i++) {
        HashTable *ht = counter_results[i]->hashtable;
        for (int j = 0; j < 256; j++) {
            WordCount *entry = ht->entries[j];
            while (entry != NULL) {
                insert_word(merged_hashtable, entry->word, entry->count);
                entry = entry->next;
            }
        }
    }

    // Find the global maximum count
    int global_max_count = get_max_count(merged_hashtable);

    // Find the word with the maximum count
    char *max_word = NULL;
    int max_count = 0;
    for (int i = 0; i < 256; i++) {
        WordCount *entry = merged_hashtable->entries[i];
        while (entry != NULL) {
            if (entry->count == global_max_count && entry->count > max_count) {
                max_word = entry->word;
                max_count = entry->count;
            }
            entry = entry->next;
        }
    }

    // Print all words with the maximum count
    for (int i = 0; i < 256; i++) {
        WordCount *entry = merged_hashtable->entries[i];
        while (entry != NULL) {
            if (entry->count == global_max_count) {
                printf("%s %d\n", entry->word, entry->count);
            }
            entry = entry->next;
        }
    }

    // Free the queues
    for (int i = 0; i < 4; i++) {
        free(queues[i]);
    }

    return 0;
}