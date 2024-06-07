#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;

atomic<int> x, y;

void *thread( void *arg ) {
  int ticket = y.fetch_add( 1, memory_order_seq_cst );
  // busy wait
  while( x.load( memory_order_relaxed ) != ticket );
  // we acquired the lock

  // critical section

  // release the lock
  x.store( ticket + 1, memory_order_relaxed );

  return NULL;
}

int main(int argc, char **argv) {
  x = 0;
  y = 0;

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
