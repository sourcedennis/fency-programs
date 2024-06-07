/// Peterson's mutex algorithm:
/// http://en.wikipedia.org/wiki/Peterson's_algorithm
///
/// The original `peterson-tso.tpl` considers only two threads, for which the
/// algorithm was designed. However, we consider the generalization for multiple
/// threads. (From "The Art of Multiprocessor Programming")

#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;


#define NUMREADERS 1
#define N 1000


long int num_threads;
// per thread, stores its level
atomic<int> level[N];
// per level, stores the last thread entering it
atomic<int> last_to_enter[N-1];

// exists k != i, such that level[ k ] >= l
bool is_anybody_in_level( int l, int i ) {
  for( int k = 0; k < num_threads - 1; k++ ) {
    if( k != i ) {
      if( level[ k ].load( memory_order_relaxed ) >= l ) {
        return true;
      }
    }
  }
  return false;
}

void* threadA(void *arg) {
  long int i = (long int) arg; // thread ID
  // First, try to acquire the lock
  for( int l = 0; l < num_threads - 1; l++ ) {
    level[ i ].store( l, memory_order_relaxed );
    last_to_enter[ l ].store( i, memory_order_relaxed );
    atomic_thread_fence(memory_order_seq_cst);

    // busy wait
    while ( last_to_enter[ l ].load( memory_order_relaxed ) == i
            && is_anybody_in_level( l, i ) ) {
    }
  }
  // we acquired the lock

  // critical section

  // release the lock now
  level[ i ].store( -1, memory_order_relaxed );

  return NULL;
}

int main(int argc, char **argv) {
  int n = atoi(argv[1]);
  num_threads = n;
  pthread_t A[ n ];

  for ( long int i = 0; i < n; i++ ) {
    level[ i ] = -1;
  }

  for (long int i = 0; i < n; i++) {
    pthread_create(&A[i], NULL, threadA, (void *)i);
  }

  for (long int i = 0; i < n; i++) {
    pthread_join(A[i], nullptr);
  }

  return 0;
}
