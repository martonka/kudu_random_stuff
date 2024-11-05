#include <atomic>
#include <iostream>

using namespace std;

struct Node {
   int value_;
   atomic<Node*> next;
};

struct MyAppendOnlyContainer {
    atomic<Node*> first{nullptr};
    atomic<Node*>* last{&first};
    atomic<size_t> num_elements{0};


    // add only called from 1 thread, so no concurrent calls.
    void add_value(int x) {
        Node* n = new Node();
        n->value_ = x;
        last->store(n, memory_order::memory_order_release);
        last = &n->next;
        // std::atomic_thread_fence(std::memory_order_release); <-- this is required
        num_elements.fetch_add(1, memory_order::memory_order_relaxed);
    }

    void print() {
        int size = num_elements;
        Node* it = first.load(memory_order::memory_order_relaxed);
        for(int i = 0; i<size; ++i) {
            cout << it->value_ << std::endl;
            it = it->next.load(memory_order_relaxed);
        }

    }

};
