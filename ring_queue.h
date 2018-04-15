/*---------------------------------------------------------------------------------------------
*  Copyright (c) Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <cstdlib>
#include <cassert>
#include <algorithm>

template<class T>
class ring_queue
{
    T* _data;
    size_t _capacity;
    size_t _first;
    size_t _count;

public:
    constexpr static size_t MinCapacity = 4;

    class iterator
    {
        friend class ring_queue;
        ring_queue *_queue;
        size_t _index;

    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef size_t difference_type;
        typedef difference_type distance_type;
        typedef T* pointer;
        typedef T& reference;

        inline iterator() : _queue(NULL), _index(0)
        {
        }

        inline iterator(ring_queue *rb, size_t index)
            : _queue(rb), _index(index)
        {
        }

        inline iterator(const iterator &other)
            : _queue(other._queue), _index(other._index)
        {
        }

        inline iterator &operator=(const iterator &other) {
            _queue = other._queue;
            _index = other._index;
            return *this;
        }

        inline iterator operator+(size_t offset) {
            return iterator(_queue, _index + offset);
        }

        inline iterator operator-(size_t offset) {
            return iterator(_queue, _index - offset);
        }

        inline size_t operator+(const iterator &other) {
            return _index + other._index;
        }

        inline size_t operator-(const iterator &other) {
            return _index - other._index;
        }

        inline iterator operator+=(size_t offset) {
            _index += offset;
            return  *this;
        }

        inline iterator operator-=(size_t offset) {
            _index -= offset;
            return  *this;
        }

        inline iterator& operator++() {
            ++_index;
            return *this;
        }

        inline iterator operator++(int) {
            iterator tmp = *this;
            ++_index;
            return tmp;
        }

        inline iterator& operator--() {
            --_index;
            return *this;
        }

        inline iterator operator--(int) {
            iterator tmp = *this;
            --_index;
            return tmp;
        }

        inline bool operator==(const iterator& other) const { return _index == other._index; }
        inline bool operator!=(const iterator& other) const { return _index != other._index; }
        inline bool operator<(const iterator &other) const { return _index < other._index; }
        inline bool operator>(const iterator &other) const { return _index > other._index; }
        inline bool operator<=(const iterator &other) const { return _index <= other._index; }
        inline bool operator>=(const iterator &other) const { return _index >= other._index; }

        inline reference operator*() const { return _queue->at(_index); }
        inline pointer operator->() const { return &_queue->at(_index); }
        inline reference operator[](size_t offset)    const { return _queue->at(_index + offset); }
    };

    class const_iterator
    {
        friend class ring_queue;
        const ring_queue *_queue;
        size_t _index;
    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef const T value_type;
        typedef size_t difference_type;
        typedef difference_type distance_type;
        typedef const T* pointer;
        typedef const T& reference;

        inline const_iterator() : _queue(nullptr), _index(0)
        {
        }

        inline const_iterator(const ring_queue *rb, size_t index)
            : _queue(rb), _index(index)
        {
        }

        inline const_iterator(const const_iterator &other)
            : _queue(other._queue), _index(other._index) {}

        inline const_iterator(const iterator &other)
            : _queue(other._queue), _index(other._index) {}

        inline const_iterator &operator=(const const_iterator &other) {
            _queue = other._queue;
            _index = other._index;
            return *this;
        }

        inline const_iterator operator+(size_t offset) {
            return const_iterator(_queue, _index + offset);
        }

        inline const_iterator operator-(size_t offset) {
            return const_iterator(_queue, _index - offset);
        }

        inline size_t operator+(const const_iterator &other) {
            return _index + other._index;
        }

        inline size_t operator-(const const_iterator &other) {
            return _index - other._index;
        }

        inline const_iterator operator+=(size_t offset) {
            _index += offset;
            return  *this;
        }

        inline const_iterator operator-=(size_t offset) {
            _index -= offset;
            return  *this;
        }

        inline const_iterator& operator++() {
            ++_index;
            return *this;
        }

        inline const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++_index;
            return tmp;
        }

        inline const_iterator& operator--() {
            --_index;
            return *this;
        }

        inline const_iterator operator--(int) {
            const_iterator tmp = *this;
            --_index;
            return tmp;
        }

        inline bool operator==(const const_iterator& other) const { return _index == other._index; }
        inline bool operator!=(const const_iterator& other) const { return _index != other._index; }
        inline bool operator<(const const_iterator &other) const { return _index < other._index; }
        inline bool operator>(const const_iterator &other) const { return _index > other._index; }
        inline bool operator<=(const const_iterator &other) const { return _index <= other._index; }
        inline bool operator>=(const const_iterator &other) const { return _index >= other._index; }

        inline reference operator*() const { return _queue->at(_index); }
        inline pointer operator->() const { return &_queue->at(_index); }
        inline reference operator[](size_t offset) const { return _queue->at(_index + offset); }
    };

    inline iterator begin() { return iterator(this, 0); }
    inline const_iterator begin() const { return const_iterator(this, 0); }
    inline const_iterator cbegin() const { return const_iterator(this, 0); }

    inline iterator end() { return iterator(this, _count); }
    inline const_iterator end()const { return const_iterator(this, _count); }
    inline const_iterator cend()const { return const_iterator(this, _count); }

    ring_queue()
        : _data(nullptr),
        _capacity(0),
        _first(0),
        _count(0)
    {
    }

    ring_queue(size_t capacity)
        : _data((T*)::operator new(capacity * sizeof(T))),
        _capacity(capacity),
        _first(0),
        _count(0)
    {
    }

    ring_queue(const ring_queue& other)
        : ring_queue(other._capacity)
    {
        for(auto& val : other)
            push_back(val);
    }

    ring_queue(ring_queue&& other)
        : _data(other._data),
        _capacity(other._capacity),
        _first(other._first),
        _count(other._count)
    {
        other._data = nullptr;
        other._capacity = 0;
        other._first = 0;
        other._count = 0;
    }

    ~ring_queue()
    {
        if(_data)
        {
            for(size_t i = 0; i < _count; ++i)
                at(i).~T();

            ::operator delete(_data);
        }
    }

    ring_queue& operator=(const ring_queue& other)
    {
        clear();
        reserve(other._count);

        for(auto& val : other)
            push_back(val);

        return *this;
    }

    ring_queue& operator=(ring_queue&& other) {
        swap(other);
        return *this;
    }

    void swap(ring_queue& other)
    {
        std::swap(_data, other._data);
        std::swap(_capacity, other._capacity);
        std::swap(_first, other._first);
        std::swap(_count, other._count);
    }

    void reserve(size_t request)
    {
        if(request < _capacity)
            return;

        size_t cap = MinCapacity;
        while(cap < request)
            cap <<= 1;

        if(_capacity == 0)
        {
            _data = (T*)::operator new(cap * sizeof(T));
            _capacity = cap;
        }
        else
        {
            ring_queue rq(cap);

            for(auto& val : *this)
                rq.push_back(move(val));

            swap(rq);
        }
    }

    template<class S>
    void push_back(S&& value)
    {
        reserve(_count + 1);
        T *p = _data + ((_first + _count) % _capacity);
        new (p) T(std::forward<S>(value));
        ++_count;
    }

    template<class InputIterator>
    void push_back(InputIterator first, InputIterator last)
    {
        while(first != last)
            push_back(*first++);
    }

    void pop_back()
    {
        T *p = _data + ((_first + _count - 1) % _capacity);
        p->~T();
        --_count;
    }

    template<class S>
    void push_front(S&& value)
    {
        reserve(_count + 1);
        _first = (_first + _capacity - 1) % _capacity;
        new (_data + _first) T(std::forward<S>(value));
        ++_count;
    }

    void pop_front()
    {
        T *p = _data + _first;
        _first = ++_first % _capacity;
        p->~T();
        --_count;
    }

    template<class InputIterator>
    void assign(InputIterator first, InputIterator last)
    {
        clear();

        while(first != last)
            push_back(*first++);
    }

    iterator erase(const iterator& it)
    {
        if(it._index < _count / 2)
        {
            auto curr = it;
            
            while(curr != begin()) {
                auto prev = curr - 1;
                *curr = std::move(*prev);
                curr = prev;
            }

            pop_front();
        }
        else
        {
            auto curr = it;
            auto next = it + 1;

            while(next != end()) {
                *curr = std::move(*next);
                curr = next++;
            }

            pop_back();
        }

        return it;
    }

    void clear()
    {
        for(size_t i = 0; i < _count; ++i)
            at(i).~T();

        _first = 0;
        _count = 0;
    }

    T& operator[](size_t index) {
        return *(_data + ((_first + index) % _capacity));
    }

    const T& operator[](size_t index) const {
        return *(_data + ((_first + index) % _capacity));
    }

    T& at(size_t index) {
        return *(_data + (_first + index) % _capacity);
    }

    const T& at(size_t index) const {
        return *(_data + ((_first + index) % _capacity));
    }

    T& back() {
        return *(_data + ((_first + _count - 1) % _capacity));
    }

    const T& back() const {
        return *(_data + ((_first + _count - 1) % _capacity));
    }

    T& front() {
        return *(_data + _first);
    }

    const T& front() const {
        return *(_data + _first);
    }

    size_t capacity() {
        return _capacity;
    }

    size_t size() const {
        return _count;
    }

    bool empty() const {
        return _count == 0;
    }
};
