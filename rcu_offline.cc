#include <iostream>
#include <atomic>
#include <cstdlib>
#include <cassert>
#include "pthread.h"

using namespace std;

// maximum number of threads
#define N 1000
// number of random actions per thread
#define NUM_ITER 30

long int num_threads;

// we have 2 cells. one is active at a time
std::atomic<bool> n_0, n_1;
// `m` marks which cell is active (false = n_0, true = n_1)
std::atomic<bool> m;
// a global lock
std::atomic<bool> L;

// global clock.
std::atomic<int> gc;
// markers for each thread. 0 if offline, otherwise 0<r<=gc
std::atomic<int> r[N];

void* thread(void *arg) {
  long int tid = (long int) arg;

  unsigned int rand_state = time(NULL) ^ tid;

  bool is_online = false;
  int cnt = 0;

  for ( int i = 0; i < NUM_ITER; i++ ) {
    int val = rand_r( &rand_state ) % 5;

    if( val == 0 ) { // read data
      if( is_online ) {
        bool cell = m.load( memory_order_relaxed );
        bool data;
        if ( cell == false ) {
          data = n_0.load( memory_order_relaxed );
        } else {
          data = n_1.load( memory_order_relaxed );
        }
        assert( data );
      }
    } else if ( val == 1 ) { // rcu_quiescent_state
      if( is_online ) {
        int a = gc.load( memory_order_relaxed );
        r[ tid ].store( a, memory_order_relaxed );
        atomic_thread_fence(memory_order_seq_cst);
      }
    } else if ( val == 2 ) { // go offline
      if( is_online ) {
        r[ tid ].store( 0, memory_order_relaxed );
        atomic_thread_fence(memory_order_seq_cst);
        is_online = false;
      }
    } else if ( val == 3 ) { // go online
      if( !is_online ) {
        int a = gc.load( memory_order_relaxed );
        r[ tid ].store( a, memory_order_relaxed );
        atomic_thread_fence(memory_order_seq_cst);
        is_online = true;
      }
    } else { // act as a writer and synchronize_rcu
      cnt = cnt + 1;
      // if online, go offline
      if( is_online ) {
        r[ tid ].store( 0, memory_order_seq_cst );
      }
      // lock
      bool old = false;
      while(!L.compare_exchange_strong(old,true,memory_order_acquire)) {
        // if it fails, it stores `true` in `old`. hence, put `false` back
        old = false;
      }

      bool cell = m.load( memory_order_relaxed );

      // "allocate" new cell
      if( cell == false ) { // n_0 was previously active
        n_1.store( true, memory_order_relaxed );
      } else { // n_1 was previously active
        n_0.store( true, memory_order_relaxed );
      }

      // tell readers to start using new cell
      m.store( !cell, memory_order_relaxed );
      // wait for all readers
      int a = gc.load( memory_order_relaxed );
      a = a + 1;
      gc.store( a, memory_order_relaxed );
      atomic_thread_fence(memory_order_seq_cst);
      for(int j=0;j<num_threads;j++){
        int local_r;
        do {
          local_r = r[j].load(memory_order_relaxed);
        } while (local_r != 0 && local_r != a);
      }
      // "deallocate" the old cell
      if( cell == false ) {
        n_0.store( false, memory_order_relaxed );
      } else {
        n_1.store( false, memory_order_relaxed );
      }
      // unlock
      L.store(false, memory_order_release);
      atomic_thread_fence(memory_order_seq_cst);
      // return online, if was online
      if( is_online ) {
        int a = gc.load( memory_order_relaxed );
        r[ tid ].store( a, memory_order_relaxed );
        atomic_thread_fence(memory_order_seq_cst);
      }
    }
  }
  // when we're done. go offline
  if( is_online ) {
    r[ tid ].store( 0, memory_order_release );
  }
  return nullptr;
}

int main(int argc, char **argv) {
  num_threads = atoi(argv[1]);

  n_0 = true;
  n_1 = false;
  m = false;
  L = false;
  gc = 1;
  for(long int i=0; i < num_threads; i++) {
    r[ i ] = 0;
  }

  pthread_t A[num_threads];

  for(long int i=0; i < num_threads; i++){
    pthread_create(&A[i], NULL, thread, (void *) i );
  }

  for(int i=0; i<num_threads; i++){
    pthread_join(A[i], nullptr);
  }

	return 0;
}
