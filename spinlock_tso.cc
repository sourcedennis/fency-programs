#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;


// our global lock
atomic<bool> x;

void *thread( void *arg ) {
  bool is_locked;

  while (true) {
    // if `x` = false, then replace it with true
    is_locked = false;
    while(!x.compare_exchange_strong( is_locked, true, memory_order_seq_cst )) {
      // if it fails, it stores `true` in `is_locked`. put `false` back
      is_locked = false;
    }

    if ( !is_locked ) {
      break;
    } else {
      // busy wait until it unlocks
      do {
        is_locked = x.load(memory_order_acquire);
      } while (is_locked);
    }
  }
  // we acquired the lock
  
  // critical section here

  // release the lock
  x.store( false, memory_order_release );

  return NULL;
}

int main(int argc, char **argv) {
  x = false;

  int n = atoi(argv[1]);

  pthread_t A[n];

  for (long int i = 0; i < n; i++) {
    pthread_create( &A[ i ], nullptr, thread, nullptr );
  }

  for (long int i = 0; i < n; i++) {
    pthread_join( A[ i ], nullptr );
  }

  return 0;
}
