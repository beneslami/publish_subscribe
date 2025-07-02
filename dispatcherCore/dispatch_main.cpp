#include <pthread.h>

extern dispatcherMain();

int main(int argc, char **argv) {
    dispatcherMain();
    pthread_exit(0);
}