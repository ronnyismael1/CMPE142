#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct Node {
    char *word;
    struct Node *next;
} Node;
typedef struct Queue {
    Node *head;
    Node *tail;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} Queue;
void init_queue(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
}
void enqueue(Queue *q, const char *word) {
    Node *new_node = NULL;

    if (word) { // Only create a new node if the word is not NULL
        new_node = malloc(sizeof(Node));
        new_node->word = strdup(word); // Duplicate the word for the queue
        new_node->next = NULL;
    }

    pthread_mutex_lock(&q->lock);

    if (q->tail != NULL) {
        q->tail->next = new_node;
    } else {
        q->head = new_node;
    }

    if (new_node) { // If it's not a sentinel, update the tail
        q->tail = new_node;
    }

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}
char *dequeue(Queue *q) {
    pthread_mutex_lock(&q->lock);

    while (q->head == NULL) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    Node *temp = q->head;
    char *word = temp->word;
    q->head = q->head->next;

    if (q->head == NULL) {
        q->tail = NULL;
    }

    pthread_mutex_unlock(&q->lock);

    free(temp); // Free the node, not the word
    return word;
}
void destroy_queue(Queue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->cond);

    // Free any remaining nodes in the queue
    Node *current = q->head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->word);  // Free the duplicated string
        free(current);
        current = next;
    }
}
void process_file(const char *filename, Queue *queues) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror(filename);
        return;
    }

    char *word = NULL;
    while (fscanf(file, "%ms", &word) == 1) {
        // Use the bottom bit of the first character to select the queue
        int queue_index = (word[0] & 1); // 0 for even, 1 for odd
        enqueue(&queues[queue_index], word);
        free(word); // Free the word after enqueuing
    }

    // Enqueue a sentinel NULL value to indicate no more words will come for each file
    for (int i = 0; i < 2; ++i) {
        enqueue(&queues[i], NULL);
    }

    fclose(file);
}

void *print_words_from_queue(void *arg) {
    Queue *queue = (Queue *)arg;

    char *word;
    while ((word = dequeue(queue)) != NULL) {
        printf("%s\n", word);
        free(word); // Free the word after printing
    }

    return NULL;
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    // Initialize 2 queues
    Queue queues[2];
    for (int i = 0; i < 2; i++) {
        init_queue(&queues[i]);
    }

    // For now, just process the first file with the queues
    process_file(argv[1], queues);

    // Create thread for each queue
    pthread_t print_threads[2];
    for (int i = 0; i < 2; ++i) {
        if (pthread_create(&print_threads[i], NULL, print_words_from_queue, &queues[i])) {
            perror("Failed to create print thread");
            // Handle error as appropriate
        }
    }

    // Join threads after they are done
    for (int i = 0; i < 2; ++i) {
        if (pthread_join(print_threads[i], NULL)) {
            perror("Failed to join print thread");
            // Handle error as appropriate
        }
    }

    // Destroy the queues after processing is done
    for (int i = 0; i < 2; i++) {
        destroy_queue(&queues[i]);
    }

    return 0;
}
