#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

#define MAX_ITER 5

using namespace std;

// global counter
atomic_int seq;
// data
atomic_int x, y;

void *write(void *arg) {
  long int d = (long int) arg;

  for( int i = 0; i < MAX_ITER; i++ ) {
    long int c = seq.load( memory_order_relaxed );
    seq.store( c+1, memory_order_relaxed );
    x.store( d, memory_order_relaxed );
    y.store( d, memory_order_relaxed );
    seq.store( c+2, memory_order_relaxed );
  }
  return NULL;
}

void *read(void *arg) {
  for( int i = 0; i < MAX_ITER; i++ ) {
    int c1 = seq.load( memory_order_relaxed );
    if ( ( c1 & 1 ) != 0 ) continue; // reread
    long int d1 = x.load( memory_order_relaxed );
    long int d2 = y.load( memory_order_relaxed );
    int c2 = seq.load( memory_order_relaxed );
    if( c1 != c2 ) continue; // reread

    if ( d1 != d2 ) {
      // error
    }
  }
  return NULL;
}

int main(int argc, char **argv) {
  seq = 0;
  x = 0;
  y = 0;

  long int n = atoi(argv[1]);

  pthread_t A[n], B[n];

  for (long int i = 0; i < n; i++) {
    pthread_create( &A[i], NULL, write, (void *) i );
    pthread_create( &B[i], NULL, read, nullptr );
  }

  for (long int i = 0; i < n; i++)
  {
    pthread_join(A[i], nullptr);
    pthread_join(B[i], nullptr);
  }

  return 0;
}

