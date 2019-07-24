/*---------------------------------------------------------------------------------------------
*  Copyright (c) 2019 Nicolas Jinchereau. All rights reserved.
*  Licensed under the MIT License. See License.txt in the project root for license information.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <vector>
#include <algorithm>
#include <type_traits>
#include <functional>

template<
    class T,
    class Cont = std::vector<T>,
    class Comp = std::less<typename Cont::value_type>
>
class PriorityQueue
{
public:
    using ValueType      = typename Cont::value_type;
    using SizeType       = typename Cont::size_type;
    using Reference      = typename Cont::reference;
    using ConstReference = typename Cont::const_reference;

    static_assert(std::is_same_v<T, ValueType>, "container adaptors require consistent types");

    PriorityQueue() noexcept(std::is_nothrow_default_constructible_v<Cont>&&
        std::is_nothrow_default_constructible_v<Comp>)
        : container(), comp()
    {
    }

    explicit PriorityQueue(const Comp& pred) noexcept(
        std::is_nothrow_default_constructible_v<Cont>&& std::is_nothrow_copy_constructible_v<Comp>)
        : container(), comp(pred)
    {
    }

    PriorityQueue(const Comp& pred, const Cont& cont)
        : container(cont), comp(pred)
    {
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Iterator>
    PriorityQueue(Iterator first, Iterator last)
        : container(first, last), comp() {
        std::make_heap(container.begin(), container.end(), comp);
    }

    template<class Iterator>
    PriorityQueue(Iterator first, Iterator last, const Comp& pred)
        : container(first, last), comp(pred) {
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Iterator>
    PriorityQueue(Iterator first, Iterator last, const Comp& pred, const Cont& cont)
        : container(cont), comp(pred) {
        container.insert(container.end(), first, last);
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Alloc, class = std::enable_if_t<std::uses_allocator_v<Cont, Alloc>>>
    explicit PriorityQueue(const Alloc& alloc) noexcept(std::is_nothrow_constructible_v<Cont, const Alloc&>&&
        std::is_nothrow_default_constructible_v<Comp>) // strengthened
        : container(alloc), comp()
    {
    }

    template <class Alloc, class = std::enable_if_t<std::uses_allocator_v<Cont, Alloc>>>
    PriorityQueue(const Comp& pred, const Alloc& alloc)
        noexcept(std::is_nothrow_constructible_v<Cont, const Alloc&>&&
            std::is_nothrow_copy_constructible_v<Comp>)
        : container(alloc), comp(pred)
    {
    }

    template <class Alloc, class = std::enable_if_t<std::uses_allocator_v<Cont, Alloc>>>
    PriorityQueue(const Comp& pred, const Cont& cont, const Alloc& alloc)
        : container(cont, alloc), comp(pred)
    {
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Alloc, class = std::enable_if_t<std::uses_allocator_v<Cont, Alloc>>>
    PriorityQueue(const PriorityQueue& rhs, const Alloc& alloc)
        : container(rhs.container, alloc), comp(rhs.comp)
    {
    }

    PriorityQueue(const Comp& pred, Cont&& cont)
        : container(std::move(cont)), comp(pred)
    {
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Iterator>
    PriorityQueue(Iterator first, Iterator last, const Comp& pred, Cont&& cont)
        : container(std::move(cont)), comp(pred)
    {
        container.insert(container.end(), first, last);
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Alloc, class = std::enable_if_t<std::uses_allocator_v<Cont, Alloc>>>
    PriorityQueue(const Comp& pred, Cont&& cont, const Alloc& alloc)
        : container(std::move(cont), alloc), comp(pred)
    {
        std::make_heap(container.begin(), container.end(), comp);
    }

    template <class Alloc, class = std::enable_if_t<std::uses_allocator_v<Cont, Alloc>>>
    PriorityQueue(PriorityQueue&& rhs, const Alloc& alloc)
        noexcept(std::is_nothrow_constructible_v<Cont, Cont, const Alloc&>&&
            std::is_nothrow_move_constructible_v<Comp>)
        : container(std::move(rhs.container), alloc), comp(std::move(rhs.comp))
    {
    }

    void push(ValueType&& val)
    {
        container.push_back(std::move(val));
        std::push_heap(container.begin(), container.end(), comp);
    }

    bool remove(const T& value)
    {
        auto it = std::find(container.begin(), container.end(), value);

        if (it != container.end()) {
            container.erase(it);
            std::make_heap(container.begin(), container.end(), comp);
            return true;
        }
        else {
            return false;
        }
    }

    template <class... V>
    void emplace(V&&... val) {
        container.emplace_back(std::forward<V>(val)...);
        std::push_heap(container.begin(), container.end(), comp);
    }

    bool empty() const {
        return container.empty();
    }

    SizeType size() const {
        return container.size();
    }

    Reference top() {
        return container.front();
    }

    ConstReference top() const {
        return container.front();
    }

    void push(const ValueType& val) {
        container.push_back(val);
        std::push_heap(container.begin(), container.end(), comp);
    }

    void pop() {
        std::pop_heap(container.begin(), container.end(), comp);
        container.pop_back();
    }

    void clear() {
        container.clear();
    }

    void swap(PriorityQueue& rhs) {
        std::swap(container, rhs.container);
        std::swap(comp, rhs.comp);
    }

    template<class Fun>
    void unordered_enumerate(const Fun& fun) {
        for (auto& e : container)
            fun(e);
    }
protected:
    Cont container;
    Comp comp;
};
