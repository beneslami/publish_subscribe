#include <pthread.h>

extern void dispatcherMain();

int main(int argc, char **argv) {
    dispatcherMain();
    pthread_exit(0);
    return 0;
}