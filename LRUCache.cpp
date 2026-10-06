#include <iostream>
#include <memory>
#include <unordered_map>

using namespace std;

class LRUCache
{
private:

    struct Node
    {
        int key;
        int value;

        shared_ptr<Node> next;
        weak_ptr<Node> prev;

        Node(int k, int v)
            : key(k), value(v)
        {
        }
    };

    int capacity;
    int size;

    shared_ptr<Node> head;
    weak_ptr<Node> tail;

    unordered_map<int, shared_ptr<Node>> cache;


    // Add node at HEAD
    void addNode(shared_ptr<Node> node)
    {
        if (!head)
        {
            // First node
            head = node;
            tail = node;
            return;
        }

        node->next = head;
        head->prev = node;

        head = node;
    }


    // Delete a node from the list
    void deleteNode(shared_ptr<Node> node)
    {
        auto prev = node->prev.lock();
        auto next = node->next;

        // Update previous node
        if (prev)
        {
            prev->next = next;
        }
        else
        {
            // Node is HEAD
            head = next;
        }

        // Update next node
        if (next)
        {
            next->prev = prev;
        }
        else
        {
            // Node is TAIL
            tail = prev;
        }

        node->next.reset();
        node->prev.reset();
    }


public:

    LRUCache(int capacity)
        : capacity(capacity), size(0)
    {
    }


    int get(int key)
    {
        auto it = cache.find(key);

        // Key doesn't exist
        if (it == cache.end())
        {
            return -1;
        }

        auto node = it->second;

        // Remove from current position
        deleteNode(node);

        // Add at HEAD
        addNode(node);

        return node->value;
    }


    void put(int key, int value)
    {
        // Check if key already exists
        auto it = cache.find(key);

        if (it != cache.end())
        {
            auto node = it->second;

            // Update value
            node->value = value;

            // Move to HEAD
            deleteNode(node);
            addNode(node);

            return;
        }


        // Cache is full
        if (size == capacity)
        {
            auto last = tail.lock();

            // Remove LRU node from map
            cache.erase(last->key);

            // Remove LRU node from list
            deleteNode(last);

            size--;
        }


        // Create new node
        auto node = make_shared<Node>(key, value);

        // Add at HEAD
        addNode(node);

        // Add to hash map
        cache[key] = node;

        size++;
    }
};


int main()
{
    LRUCache cache(3);

    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);

    cout << cache.get(1) << endl;

    cache.put(4, 400);

    cout << cache.get(2) << endl;

    return 0;
}
