#include <pthread.h>
#include <cstdlib>
#include <unistd.h>

#include "../include/async.hpp"
#include "../include/shared_prefs.hpp"

struct AsyncTask {
    SharedPreferences* sp;
    AsyncTask* next;

    AsyncTask(SharedPreferences* s) : sp(s), next(NULL) {}
};

struct AsyncWorker {
private:
    pthread_t worker;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    AsyncTask* head;
    AsyncTask* tail;
    bool running;

    static void* worker_loop(void* arg) {
        AsyncWorker* self = (AsyncWorker*)arg;

        while (self->running) {
            pthread_mutex_lock(&self->lock);

            while (!self->head && self->running)
                pthread_cond_wait(&self->cond, &self->lock);

            if (!self->running) {
                pthread_mutex_unlock(&self->lock);
                break;
            }

            AsyncTask* task = self->head;
            self->head = task->next;

            if (!self->head)
                self->tail = NULL;

            pthread_mutex_unlock(&self->lock);
            // Perform the async operation (flush to disk)
            
            sleep(MAX_DELAY_SECS);
            
            pthread_mutex_lock(&task->sp->lock);

            if(task->sp->dirty) {
                Snapshot* snap = new Snapshot(task->sp->map);
                task->sp->dirty=false;

                pthread_mutex_unlock(&task->sp->lock);

                task->sp->storage->flush(snap->copy);
                delete snap;
            }
            else {
                pthread_mutex_unlock(&task->sp->lock);
            }

            delete task;
        }

        return NULL;
    }

public:
    void init() {
        head = NULL;
        tail = NULL;
        running = true;

        pthread_mutex_init(&lock, NULL);
        pthread_cond_init(&cond, NULL);
        pthread_create(&worker, NULL, worker_loop, this);
    }

    void destroy() {
        pthread_mutex_lock(&lock);
        running = false;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);
        
        pthread_join(worker, NULL);
        
        // Clean up any tasks still remaining in the queue to prevent leaks
        AsyncTask* current = head;
        while (current) {
            AsyncTask* next = current->next;
            current = next;

            delete current;
        }
        head = NULL;
        tail = NULL;

        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
    }

    void schedule(SharedPreferences* sp) {
        AsyncTask* task = new AsyncTask(sp);

        pthread_mutex_lock(&lock);

        if (!head)
            head = task;
        else
            tail->next = task;

        tail = task;

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
};

static AsyncWorker* worker = NULL;

void async_init() {
    if (!worker) {
        worker = new AsyncWorker();
        worker->init();
    }
}

void async_schedule(SharedPreferences* sp) {
    if (worker) {
        worker->schedule(sp);
    }
}

void async_shutdown() {
    if (worker) {
        worker->destroy(); // Joins thread and clears leftover tasks
        delete worker;
        worker = NULL;
    }
}