#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/time.h>

#include "../include/async.hpp"
#include "../include/shared_prefs.hpp"

const int FLUSH_DELAY_MS = 1000;

struct AsyncTask {
    SharedPreferences* sp;
    AsyncTask* next;
    WriteStrategy strategy;  // true for commit(), false for apply()

    AsyncTask(SharedPreferences* s, WriteStrategy strategy = WriteStrategy::APPLY) : sp(s), next(nullptr), strategy(strategy) {}
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
        struct timeval tv;
        struct timespec ts;

        while (self->running) {
            pthread_mutex_lock(&self->lock);

            // Wait with timeout for tasks
            if (!self->head && self->running) {
                gettimeofday(&tv, nullptr);
                ts.tv_sec = tv.tv_sec + (FLUSH_DELAY_MS / 1000);
                ts.tv_nsec = (tv.tv_usec + (FLUSH_DELAY_MS % 1000) * 1000) * 1000;
                pthread_cond_timedwait(&self->cond, &self->lock, &ts);
            }

            if (!self->running) {
                pthread_mutex_unlock(&self->lock);
                break;
            }

            // Batch process all available tasks
            AsyncTask* batch = self->head;
            self->head = nullptr;
            self->tail = nullptr;

            pthread_mutex_unlock(&self->lock);

            // Process all tasks in batch without holding lock
            while (batch) {
                AsyncTask* task = batch;
                batch = task->next;
                task->next = nullptr;
                
                // Lock only for accessing dirty flag and snapshot
                pthread_mutex_lock(&task->sp->lock);
                bool should_flush = task->sp->dirty;
                Snapshot* snap = should_flush ? new Snapshot(task->sp->map) : nullptr;
                task->sp->dirty = false;
                pthread_mutex_unlock(&task->sp->lock);

                if (should_flush && snap) {
                    if (task->strategy == WriteStrategy::COMMIT) {
                        task->sp->storage->flush_atomic(snap->copy);
                    } else {
                        task->sp->storage->flush(snap->copy);
                    }
                    delete snap;
                }
                
                if (task->strategy == WriteStrategy::COMMIT) {
                    task->sp->notify_commit_done();
                }

                delete task;
            }
        }

        return nullptr;
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
        
        pthread_join(worker, nullptr);
        
        // Clean up any tasks still remaining in the queue to prevent leaks
        AsyncTask* current = head;
        while (current) {
            AsyncTask* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
        tail = nullptr;

        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&cond);
    }

    void schedule(SharedPreferences* sp, WriteStrategy strategy = WriteStrategy::APPLY) {
        AsyncTask* task = new AsyncTask(sp, strategy);

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

static AsyncWorker* worker = nullptr;
static int ref_count = 0;
static pthread_mutex_t ref_lock = PTHREAD_MUTEX_INITIALIZER;

void async_init() {
    pthread_mutex_lock(&ref_lock);
    ref_count++;

    if (!worker) {
        worker = new AsyncWorker();
        worker->init();
    }

    pthread_mutex_unlock(&ref_lock);
}

void async_schedule(SharedPreferences* sp, WriteStrategy strategy) {
    if (worker) {
        worker->schedule(sp, strategy);
    }
}

void async_schedule_sync(SharedPreferences* sp) {
    async_schedule(sp, WriteStrategy::COMMIT);
}

void async_shutdown() {
    pthread_mutex_lock(&ref_lock);
    ref_count--;

    if (ref_count == 0 && worker) {
        worker->destroy();
        delete worker;
        worker = nullptr;
    }

    pthread_mutex_unlock(&ref_lock);
}