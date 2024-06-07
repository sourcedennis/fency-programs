#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

// number of random actions per thread
#define NUM_ITER 100

using namespace std;

atomic<int> H; // head
atomic<int> T; // tail
atomic<bool> L; // lock

void push( ) {
   int a = T.load(memory_order_relaxed);
   T.store(a + 1, memory_order_relaxed);
}

void pop( ) {
   int a = T.load(memory_order_relaxed);
   T.store(a - 1, memory_order_relaxed);
   atomic_thread_fence(memory_order_seq_cst);
   if (H.load(memory_order_relaxed) > T.load(memory_order_relaxed)) {
      int b = T.load(memory_order_relaxed);
      T.store(b + 1, memory_order_relaxed);
      // lock
      bool y = false;
      while (!L.compare_exchange_strong(y, true, memory_order_acquire)) {
         y = false;
      }
      int c = T.load(memory_order_relaxed);
      T.store(c - 1, memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
      if (H.load(memory_order_relaxed) > T.load(memory_order_relaxed)) {
         int e = T.load(memory_order_relaxed);
         T.store(e + 1, memory_order_relaxed);
         // failure
      } // otherwise: success
      // unlock
      L.store(false, memory_order_release);
   }
}

void *owner(void *arg) {
  unsigned int rand_state = time(NULL);
  for ( int i = 0; i < NUM_ITER; i++ ) {
    int val = rand_r( &rand_state ) % 2;

    if( val == 0 ) {
      push( );
    } else { // val == 1
      pop( );
    }
  }
  return NULL;
}

void *steal(void *arg) {
   for ( int i = 0; i < NUM_ITER; i++ ) {
      // lock
      bool y = false;
      while (!L.compare_exchange_strong(y, true, memory_order_acquire)) {
         y = false;
      }
      int a = H.load(memory_order_relaxed);
      H.store(a + 1, memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
      if (H.load(memory_order_relaxed) > T.load(memory_order_relaxed)) {
         int b = H.load(memory_order_relaxed);
         H.store(b - 1, memory_order_relaxed);
         // failure
      } // otherwise: success
      // unlock
      L.store(false, memory_order_release);
   }
   return NULL;
}

int main(int argc, char **argv)
{
   pthread_t A, B;

   T = NUM_ITER;
   H = NUM_ITER;
   L = false;

   pthread_create(&A, NULL, owner, nullptr );
   pthread_create(&B, NULL, steal, nullptr );

   pthread_join( A, nullptr);
   pthread_join( B, nullptr);

   return 0;
}
