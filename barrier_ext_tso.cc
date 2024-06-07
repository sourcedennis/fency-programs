// A barrier. Threads block until all threads have reached the barrier.
// This is a generalization of `barrier.tpl` to multiple threads. Otherwise,
// the mechanism is the same, where each thread has its own signaling variable.

#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;

// maximum number of threads
#define N 1000

// The total number of threads (<=N)
unsigned int n_total;
// `true` for the threads that have reached the barrier and are now waiting
// for it
atomic<bool> is_thread_waiting[ N ];

// busy wait for `is_thread_waiting[ i ]` to become true
void wait( int thread_id ) {
  bool has_other_reached = is_thread_waiting[ thread_id ].load( memory_order_relaxed );
  while( !has_other_reached ) {
    has_other_reached = is_thread_waiting[ thread_id ].load( memory_order_relaxed );
  }
}

void *threadA( void *arg ) {
  long int thread_id = (long int) arg;

  is_thread_waiting[ thread_id ].store( true, memory_order_relaxed );
  atomic_thread_fence(memory_order_seq_cst);

  for( unsigned int i = 0; i < n_total; i++ ) {
    if( i != thread_id ) {
      // wait until `is_thread_waiting[ i ]` becomes true
      wait( i );
    }
  }
  // all threads reached the barrier

  return NULL;
}

int main(int argc, char **argv) {
  int n = atoi(argv[1]);
  n_total = n;
  pthread_t A[ n ];

  for ( long int i = 0; i < n; i++ ) {
    is_thread_waiting[ i ] = false;
  }

  for (long int i = 0; i < n; i++) {
    pthread_create(&A[i], NULL, threadA, (void *)i);
  }

  for (long int i = 0; i < n; i++) {
    pthread_join(A[i], nullptr);
  }

  return 0;
}
