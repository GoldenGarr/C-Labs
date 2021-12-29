#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define max(a, b) (((a) > (b)) ? (a) : (b))

const int LINE_SIZE = 400;


void search_5xx_errors() {
    char *token;
    char *regex = "\"";
    int counter = 0;
    int failed = 0;
    char line[LINE_SIZE];
    char saved[LINE_SIZE];

    //FILE *file = fopen("/Users/maksim/Desktop/ctester/debug.txt", "r");
    FILE *file = fopen("/Users/maksim/Desktop/prog/access_log_Jul95", "r");

    while (fgets(line, LINE_SIZE, file) != NULL) {
        strcpy(saved, line);
        token = strtok(line, regex);
        while (token != NULL) {
            ++counter;
            token = strtok(NULL, regex);
            if (counter == 2) {
                if (token[1] == '5') {
                    printf("%s\n", saved);
                    failed++;
                }
            }
        }
        counter = 0;
    }

    printf("\nFAILED: %d\n", failed);

    fclose(file);
}


void search_time_window() {
    printf("Please enter the time window in seconds: \n");
    long int window;
    scanf("%ld", &window);

    struct tm time;

    FILE *file = fopen("/Users/maksim/Desktop/prog/access_log_Jul95", "r");
    //FILE *file = fopen("/Users/maksim/Desktop/ctester/debug.txt", "r");

    int time_window;

    char *token;
    char *regex = "[";
    int count = 0;
    long size = 0;
    char line[LINE_SIZE];

    int res = -1;
    int queue_size = 25;

    long int *queue = (long int *) malloc(sizeof(long int) * queue_size);
    int head = 0;
    int tail = 1;
    int length = 0;

    long int tmp;

    while (fgets(line, LINE_SIZE, file) != NULL) {
        size++;
        token = strtok(line, regex);
        while (token != NULL) {
            count++;
            if (count == 2) {
                strptime(token, "%d/%b/%Y:%H:%M:%S", &time);

                queue[tail - 1] = mktime(&time);
                if (size == 1) {
                    queue[tail - 1] -= 3600;
                }

                tmp = queue[tail - 1];
                length++;

                if (tail++ == queue_size) {
                    tail = 0;
                }

                if (tail == queue_size) {
                    queue_size += 25;
                    queue = (long int *) realloc(queue, sizeof(long int) * queue_size);
                }

                while ((tmp - queue[head]) > window) {
                    if (++head == queue_size) {
                        head = 0;
                    }
                    length--;
                }

                res = max(res, length);
                break;
            }
            token = strtok(NULL, regex);
        }
        count = 0;
    }

    printf("MAXIMUM NUMBER OF REQUESTS IN A %ld SEC WINDOW IS %d\n", window, res);
    fclose(file);
    free(queue);
    queue = NULL;
    /*
     * cycled queue
     * ex: window_size = 2 seconds
     * given time in seconds:
     * 1 1 1 2 2 3 4
     * a = []
     * head = 0;
     * tail = 1;
     * add element -> a[tail++ - 1] = new_element
     * delete element -> head++
     * tail >= a.length -> tail = 0
     * head <= -1 -> head = a.length - 1
     * if after addition head = tail -> realloc
     *
     *
     *
     * */
}


int main() {
    search_5xx_errors();
    search_time_window();
    return 0;
}
