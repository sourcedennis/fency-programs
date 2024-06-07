#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;


#define N 1000000


int num_threads; // < N
atomic<int> x, y;
atomic<bool> b[N+1];

void* func(void *arg) {
  long int i = (long int) arg;

  while(true){
    b[i].store(true,memory_order_relaxed);
    x.store(i,memory_order_relaxed);
    atomic_thread_fence(memory_order_seq_cst);
    if(y.load(memory_order_relaxed) != 0){
      b[i].store(false,memory_order_relaxed);

      while(y.load(memory_order_acquire)!=0) { }
    } else {
      y.store(i,memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
      int a = x.load(memory_order_relaxed);
      if(a != i) {
        b[i].store(false,memory_order_relaxed); 
        for(int j=1;j<=num_threads; j++){
          while( b[j].load(memory_order_acquire) != false ) { }
        }
        if(y.load(memory_order_relaxed)==i){
          break; // goto critical section
        }
        while(y.load(memory_order_acquire)!=0) { }
      } else {
        break; // goto critical section
      }
    }
  }
  // we acquired the lock

  // critical section

  // release the lock now
  y.store(0,memory_order_release);
  b[i].store(false,memory_order_release);

  return NULL;
}


int main(int argc, char **argv) {
  int n = atoi(argv[1]);
  num_threads = n;

  pthread_t A[n+1];
  x = 0;
  y = 0;

  for(long int i=1;i<=n;i++){
    b[ i ] = false;
  }

  for(long int i=1;i<=n;i++){
    pthread_create( &A[i], NULL, func, (void *) i );
  }

  for(int i=1;i<=n;i++){
    pthread_join( A[i], nullptr );
  }

  return 0;
}
