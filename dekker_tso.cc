/*
 * Dekker's critical section algorithm, implemented with fences.
 *
 * URL:
 *   http://www.justsoftwaresolutions.co.uk/threading/
 */

#include <iostream>
#include <atomic>
#include <cstdlib>
#include "pthread.h"

using namespace std;

// flag0 indicates `p0` *wants to enter*
// flag1 indicates `p1` *wants to enter*
atomic<bool> flag0, flag1;
atomic<int> turn;

void *p0(void *arg) {
  int otherflag, localturn;

  // we want to enter
  flag0.store(true, memory_order_relaxed);
  atomic_thread_fence(memory_order_seq_cst);

  otherflag = flag1.load(memory_order_relaxed);
  while (otherflag) {
    localturn = turn.load(memory_order_relaxed);
    if (localturn != 0) {
      // we don't want to enter. busy wait
      flag0.store(false, memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
      localturn = turn.load(memory_order_relaxed);
      while (localturn != 0) {
        localturn = turn.load(memory_order_relaxed);
      }
      // now we want to enter again
      flag0.store(true, memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
    }
    // check again if the other thread wants to enter
    otherflag = flag1.load(memory_order_relaxed);
  }
  // acquired the lock

  // critical section here

  // now release the lock
  turn.store(1, memory_order_release);
  flag0.store(false, memory_order_release);

  return NULL;
}

void *p1(void *arg) {
  int otherflag, localturn;

  // we want to etner
  flag1.store(true, memory_order_relaxed);
  atomic_thread_fence(memory_order_seq_cst);

  otherflag = flag0.load(memory_order_relaxed);
  while (otherflag) {
    localturn = turn.load(memory_order_relaxed);
    if (localturn != 1) {
      // we don't want to enter. busy wait
      flag1.store(false, memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
      localturn = turn.load(memory_order_relaxed);
      while (localturn != 1) {
        localturn = turn.load(memory_order_relaxed);
      }
      // now we want to enter again
      flag1.store(true, memory_order_relaxed);
      atomic_thread_fence(memory_order_seq_cst);
    }
    // check again if the other thread wants to enter
    otherflag = flag0.load(memory_order_relaxed);
  }
  // acquired the lock

  // critical section here

  // now release the lock
  turn.store(0, memory_order_release);
  flag1.store(false, memory_order_release);

  return NULL;
}

int main(int argc, char **argv)
{
  flag0 = false;
  flag1 = false;
  turn = 0;

  pthread_t A, B;

  pthread_create(&A, NULL, p0, nullptr);
  pthread_create(&B, NULL, p1, nullptr);

  pthread_join(A, nullptr);
  pthread_join(B, nullptr);

  return 0;
}
