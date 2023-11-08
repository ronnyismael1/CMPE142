#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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

typedef struct {
    char *filename;
    Queue *queues[4];
} ReaderArgs;

typedef struct {
    Queue *queue;
} PrinterArgs;

Queue* create_queue() {
    Queue *q = malloc(sizeof(Queue));
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
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
    for (int i = 0; i < 4; i++) {
        enqueue(queues[i], NULL);  // Add sentinel value
    }
    fclose(file);
}

void *process_queue(void *arg) {
    PrinterArgs *args = (PrinterArgs *)arg;
    Queue *queue = args->queue;
    char *word;
    while ((word = dequeue(queue)) != NULL) {
        printf("%s\n", word);
        free(word);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Create 4 queues
    Queue *queues[4];
    for (int i = 0; i < 4; i++) {
        queues[i] = create_queue();
    }

    pthread_t reader_threads[argc - 1];
    ReaderArgs reader_args[argc - 1];
    for (int i = 1; i < argc; i++) {
        reader_args[i - 1].filename = argv[i];
        memcpy(reader_args[i - 1].queues, queues, sizeof(queues));
        pthread_create(&reader_threads[i - 1], NULL, enqueue_words_from_file, &reader_args[i - 1]);
    }

    pthread_t printer_threads[4];
    PrinterArgs printer_args[4];
    for (int i = 0; i < 4; i++) {
        printer_args[i].queue = queues[i];
        pthread_create(&printer_threads[i], NULL, process_queue, &printer_args[i]);
    }

    // Join reader threads
    for (int i = 0; i < argc - 1; i++) {
        pthread_join(reader_threads[i], NULL);
    }

    // Join printer threads
    for (int i = 0; i < 4; i++) {
        pthread_join(printer_threads[i], NULL);
    }

    // Free the queues
    for (int i = 0; i < 4; i++) {
        // TODO: Add code to free the nodes in the queue
        free(queues[i]);
    }

    return 0;
}