// A barrier, for two threads. Threads block until all have reached the barrier.

#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;

atomic<bool> x;
atomic<bool> y;

void *threadA( void *arg ) {
  x.store( true, memory_order_relaxed );

  atomic_thread_fence(memory_order_seq_cst);

  // busy wait on the other thread
  bool local_y;
  do {
    local_y = y.load( memory_order_relaxed );
  } while( !local_y );

  return nullptr;
}

void *threadB( void *arg ) {
  y.store( true, memory_order_relaxed );

  atomic_thread_fence(memory_order_seq_cst);

  // busy wait on the other thread
  bool local_x;
  do {
    local_x = x.load( memory_order_relaxed );
  } while( !local_x );

  return nullptr;
}

int main(int argc, char **argv) {
  x = false;
  y = false;
  
  pthread_t A, B;

  pthread_create( &A, nullptr, threadA, nullptr );
  pthread_create( &B, nullptr, threadB, nullptr );

  pthread_join( A, nullptr );
  pthread_join( B, nullptr );

  return 0;
}
