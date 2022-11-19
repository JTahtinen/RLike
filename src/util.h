#pragma once
#include <jadel.h>

template <typename T>
struct LinkedListIterator
{
    jadel::LinkedList<T>* _list;
    jadel::Node<T>* _current;

    LinkedListIterator(jadel::LinkedList<T>* list)
        : _list(list)
        , _current(list->head)
    {
    }

    T* getNext()
    {
        T* result = NULL;
        if (_current)
        {
            result = &_current->data;
            _current = _current->next;
        }
        return result;
    }
};