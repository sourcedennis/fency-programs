/// Peterson's mutex algorithm:
/// http://en.wikipedia.org/wiki/Peterson's_algorithm

#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;

atomic<int> turn;
atomic<bool> x1, x2;

void* threadA(void *arg) {
  bool x2_local;
  int turn_local;

  x1.store( true, memory_order_relaxed );
  turn.store( 1, memory_order_relaxed );
  atomic_thread_fence(memory_order_seq_cst);

  while ( true ) {
    x2_local = x2.load( memory_order_relaxed );
    if( !x2_local ) {
      break;
    }
    turn_local = turn.load( memory_order_relaxed );
    if( turn_local == 0 ) {
      break;
    }
  }

  x1.store( false, memory_order_relaxed );

	return NULL;
}

void* threadB(void *arg) {
  bool x1_local;
  int turn_local;

  x2.store( true, memory_order_relaxed );
  turn.store( 0, memory_order_relaxed );
  atomic_thread_fence(memory_order_seq_cst);

  while ( true ) {
    x1_local = x1.load( memory_order_relaxed );
    if( !x1_local ) {
      break;
    }
    turn_local = turn.load( memory_order_relaxed );
    if( turn_local == 1 ) {
      break;
    }
  }

  x2.store( false, memory_order_relaxed );

	return NULL;
}

int main(int argc, char **argv) {
  x1 = 0;
  x2 = 0;

  pthread_t A, B;

  pthread_create( &A, NULL, threadA, nullptr );
  pthread_create( &B, NULL, threadB, nullptr );

  pthread_join(A, nullptr);
  pthread_join(B, nullptr);

  return 0;
}
