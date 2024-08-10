#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <cassert>

template <typename T> class Vector {
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;

    using pointer = value_type*;
    using const_pointer = const pointer;

    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    size_type capacity_{};
    size_type size_{};

    pointer data_ = nullptr;

    template <bool is_const>
    class base_iterator {
    public:
        using pointer_type = typename std::conditional_t<is_const, const_pointer, pointer>;
        using reference_type = typename std::conditional_t<is_const, const_reference, reference>;

        pointer_type ptr_ = nullptr;
        pointer_type begin_ = nullptr;
        pointer_type end_ = nullptr;

        base_iterator() : ptr_(nullptr), begin_(nullptr), end_(nullptr) {}

        base_iterator(pointer_type ptr, pointer_type begin, pointer_type end)
            : ptr_(ptr), begin_(begin), end_(end) {}
        base_iterator(const base_iterator& other) : begin_(other.begin_), ptr_(other.begin_), end_(other.end_) {}

        base_iterator& operator=(const base_iterator& other)const {
            begin_ = other.begin_;
            ptr_ = other.ptr_;
            end_ = other.end_;
        }

        reference_type operator*() const {
            check_range();
            return *ptr_;
        }
        pointer_type operator->() const {
            check_range();
            return ptr_;
        }

        base_iterator& operator++() {
            ++ptr_;
            check_range();
            return *this;
        }

        base_iterator& operator--() {
            --ptr_;
            check_range();
            return *this;
        }

        base_iterator operator++(int) {
            base_iterator copy<false> = *this;
            ++ptr_;
            check_range();
            return copy;
        }

        base_iterator operator--(int) {
            base_iterator<false> copy = *this;
            --ptr_;
            check_range();
            return copy;
        }

        base_iterator operator+(const difference_type value) const {
            base_iterator<false> temp = *this;
            temp.ptr_ += value;
            temp.check_range();
            return temp;
        }

        base_iterator operator-(const difference_type value) const {
            base_iterator<false> temp = *this;
            temp.ptr_ -= value;
            temp.check_range();
            return temp;
        }

        difference_type operator-(const base_iterator& other) const {
            return ptr_ - other.ptr_;
        }

        base_iterator& operator+=(const difference_type value) {
            ptr_ += value;
            check_range();
            return *this;
        }

        base_iterator& operator-=(const difference_type n) {
            ptr_ -= n;
            check_range();
            return *this;
        }

        bool operator==(const base_iterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const base_iterator& other) const {
            return ptr_ != other.ptr_;
        }

        bool operator<(const base_iterator& other) const {
            return ptr_ < other.ptr_;
        }

        bool operator<=(const base_iterator& other) const {
            return ptr_ <= other.ptr_;
        }

        bool operator>(const base_iterator& other) const {
            return ptr_ > other.ptr_;
        }

        bool operator>=(const base_iterator& other) const {
            return ptr_ >= other.ptr_;
        }

        operator base_iterator<true>() const {
            return { ptr_, begin_, end_ };
        }

        operator base_iterator<false>() const {
            return { ptr_, begin_, end_ };
        }

        void check_range() const {
            if (ptr_ < begin_ || ptr_ > end_) {
                throw std::out_of_range("Iterator out of range");
            }
        }
    };

    void preparation_for_exit()
    {
        clear();
        delete[] reinterpret_cast<char*>(data_);
    }

public:
    using iterator = base_iterator<false>;
    using const_iterator = base_iterator<true>;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Vector() : capacity_(0), size_(0), data_(nullptr) {}

    Vector(size_type n) : capacity_(n), size_(n), data_(reinterpret_cast<pointer>(::new char[n * sizeof(value_type)])) {
        std::uninitialized_default_construct_n(data_, n);
    }

    Vector(size_type n, const_reference value)
        : capacity_(n), size_(n), data_(reinterpret_cast<pointer>(::new char[n * sizeof(value_type)])) {
        std::uninitialized_fill_n(data_, n, value);
    }

    Vector(const std::initializer_list<T>& list)
        : capacity_(list.size()), size_(list.size()), data_(reinterpret_cast<pointer>(::new char[capacity_ * sizeof(value_type)])) {
        std::uninitialized_copy(list.begin(), list.end(), data_);
    }

    template <class InputIt>
    Vector(InputIt first, InputIt last) {
        if (first == last) {
            return;
        }

        size_type distance = std::distance(first, last);
        reserve(distance);

        for (; first != last; ++first) {
            push_back(*first);
        }
    }

    void reserve(const size_type new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        value_type* new_arr;

        try {
            //new_arr = static_cast<T*>(operator new[](new_capacity * sizeof(T)));
            new_arr = reinterpret_cast<value_type*>(::new char[new_capacity * sizeof(T)]);
        }
        catch (const std::bad_alloc& bad) {
            std::cout << bad.what();
            throw;
        }

        try {
            std::uninitialized_copy(data_, data_ + size_, new_arr);
        }
        catch (...) {
            delete[] reinterpret_cast<char*>(new_arr);
            throw;
        }

        std::destroy(data_, data_ + size_);
        delete[] reinterpret_cast<char*>(data_);

        data_ = new_arr;
        capacity_ = new_capacity;
    }

    /*void reserve(const size_type new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        std::allocator<value_type> alloc;
        value_type* new_arr = alloc.allocate(new_capacity);

        try {
            std::uninitialized_move_n(data_, size_, new_arr);
        }
        catch (...) {
            alloc.deallocate(new_arr, new_capacity);
            throw;
        }

        std::destroy(data_, data_ + size_);
        alloc.deallocate(data_, capacity_);

        data_ = new_arr;
        capacity_ = new_capacity;
    }*/

    void push_back(const_reference element) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 1);
        }

        try {
            ::new (data_ + size_++) T(element);
        }
        catch (...) {
            (data_ + --size_)->~T();
            throw;
        }
    }

    size_type size() const noexcept { return size_; }
    size_type capacity() const noexcept { return capacity_; }

    void pop_back() { (data_ + --size_)->~T(); }

    void resize(size_type count) {
        if (count > capacity_) {
            reserve(count);
        }

        if (count == 0) {
            clear();
        }
        else {
            if (count > size_) {
                std::uninitialized_fill_n(data_ + size_, count - size_, 0);
            }
            else {
                std::destroy(data_ + count, data_ + size_);
            }
        }

        size_ = count;
    }

    void resize(size_type count, const_reference value) {
        if (count > capacity_) {
            reserve(count);
        }

        if (count == 0) {
            clear();
        }
        else {
            if (count > size_) {
                try {
                    std::uninitialized_fill_n(data_ + size_, count - size_, value);
                }
                catch (...) {
                    preparation_for_exit();
                    throw;
                }

            }
            else {
                std::destroy(data_ + count, data_ + size_);
            }
        }

        size_ = count;
    }

    iterator insert(const_iterator pos, const_reference value) {
        difference_type index = pos.ptr_ - data_;
        if (size_ >= capacity_ - 1) {
            reserve(capacity_ ? capacity_ * 2 : 1);
        }

        const_pointer ptr = data_ + index;

        if (ptr < data_ + size_) {
            std::uninitialized_copy(ptr, data_ + size_, ptr + 1);
            (ptr)->~T();
        }

        ::new (ptr) value_type(value);
        ++size_;

        return iterator(data_ + index, data_, data_ + size_);
    }

    iterator begin() noexcept { return iterator(data_, data_, data_ + size_); }
    const_iterator cbegin() const noexcept { return const_iterator(data_, data_, data_ + size_); }

    iterator end() noexcept { return iterator(data_ + size_, data_, data_ + size_); }
    const_iterator cend() const noexcept { return const_iterator(data_ + size_, data_, data_ + size_); }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

    reference front() { return *data_; }
    const_reference front() const { return *data_; }

    reference back() { return data_[size_ - 1]; }
    const_reference back() const { return data_[size_ - 1]; }

    bool empty()const { return data_ == data_ + size_; }

    void shrink_to_fit() {
        if (size_ < capacity_) {
            pointer new_data = reinterpret_cast<value_type*>(::new char[size_ * sizeof(value_type)]);
            try {
                std::uninitialized_copy(data_, data_ + size_, new_data);
            }
            catch (...) {
                delete[] reinterpret_cast<char*>(new_data);
                throw;
            }

            std::destroy(data_, data_ + size_);
            delete[] reinterpret_cast<char*>(data_);

            data_ = new_data;
            capacity_ = size_;
        }
    }

    iterator erase(iterator pos) {
        if (pos.ptr_ < data_ || pos.ptr_ >= data_ + size_) {
            throw std::out_of_range("Iterator out of range");
        }

        pointer ptr = pos.ptr_;

        for (pointer it = ptr; it + 1 < data_ + size_; ++it) {
            *it = *(it + 1);
        }

        return iterator(ptr, data_, data_ + size_--);
    }

    iterator erase(iterator first, iterator last) {
        if (first.ptr_ < data_ || first.ptr_ >= data_ + size_ || last.ptr_ < data_ || last.ptr_ > data_ + size_ || first.ptr_ > last.ptr_) {
            throw std::out_of_range("Iterator out of range");
        }

        pointer ptr_first = first.ptr_;
        pointer ptr_last = last.ptr_;

        for (pointer it = ptr_first; it + (ptr_last - ptr_first) < data_ + size_; ++it) {
            *it = *(it + (ptr_last - ptr_first));
        }

        size_ -= (ptr_last - ptr_first);

        return iterator(ptr_first, data_, data_ + size_);
    }

    iterator erase(const_iterator pos) {
        return erase(static_cast<iterator>(pos));
    }

    iterator erase(const_iterator first, const_iterator last) {
        return erase(static_cast<iterator>(first), static_cast<iterator>(last));
    }

    void clear() {
        if (size_ > 0) {
            std::destroy(data_, data_ + size_);
            size_ = 0;
        }
    }

    reference operator[](const size_type index) { return data_[index]; }
    const_reference operator[](const size_type index) const {
        return data_[index];
    }

    reference at(const size_type index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }
        return data_[index];
    }

    const_reference at(const size_type index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range");
        }

        return data_[index];
    }

    ~Vector() {
        preparation_for_exit();
    }
};

template <>
class Vector<bool> {
    struct Bit_reference {
        std::uint8_t* bucket_ptr_;
        std::uint8_t  pos_;

        Bit_reference(std::uint8_t* ptr, const std::uint8_t pos) : bucket_ptr_(ptr), pos_(pos) {}

        Bit_reference operator=(bool bit) {
            bit ? *bucket_ptr_ |= (static_cast<uint8_t>(1) << pos_) : *bucket_ptr_ &= ~(static_cast<uint8_t>(1) << pos_);
            return *this;
        }

        operator bool() const {
            return *bucket_ptr_ & (1 << pos_);
        }
    };

    uint8_t* arr;
    std::size_t size_;
    std::size_t capacity_;
public:
    Bit_reference operator[](std::size_t index) {
        std::uint8_t pos = index % 8;
        std::uint8_t* ptr = arr + index / 8;

        return Bit_reference(ptr, pos);
    }

    Vector(const std::initializer_list<bool>& list) {
        size_ = list.size();
        capacity_ = (size_ + 7) / 8;
        arr = new uint8_t[capacity_]();

        std::size_t i = 0;
        for (bool value : list) {
            if (value) {
                arr[i / 8] |= (static_cast<uint8_t>(1) << (i % 8));
            }
            ++i;
        }
    }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            delete[] arr;
            size_ = other.size_;
            capacity_ = other.capacity_;
            arr = new uint8_t[capacity_];
            std::copy(other.arr, other.arr + capacity_, arr);
        }
        return *this;
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t capacity() const {
        return capacity_;
    }

    void pop_back() {
        if (size_ == 0) {
            throw std::out_of_range("Cannot pop from an empty vector");
        }
        --size_;
        if (arr != nullptr) {
            std::size_t pos = size_ % 8;
            arr[size_ / 8] &= ~(static_cast<uint8_t>(1) << (7 - pos));
        }
    }

    void push_back(bool value) {
        if (size_ == capacity_ * 8) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        if (value) {
            arr[size_ / 8] |= (static_cast<uint8_t>(1) << (size_ % 8));
        }
        ++size_;
    }

    void reserve(std::size_t new_capacity) {
        if (new_capacity == capacity_) {
            return;
        }
        else if (new_capacity < capacity_) {
            std::size_t new_size = std::min(new_capacity, size_);
            std::size_t new_byte_capacity = (new_capacity + 7) / 8;
            uint8_t* new_arr = new uint8_t[new_byte_capacity]();

            if (arr != nullptr) {
                std::size_t current_byte_capacity = (capacity_ + 7) / 8;
                std::copy(arr, arr + std::min(current_byte_capacity, new_byte_capacity), new_arr);
                delete[] arr;
            }

            arr = new_arr;
            capacity_ = new_capacity;
            size_ = new_size;
        }
        else {
            std::size_t new_byte_capacity = (new_capacity + 7) / 8;
            uint8_t* new_arr = new uint8_t[new_byte_capacity]();

            if (arr != nullptr) {
                std::size_t current_byte_capacity = (capacity_ + 7) / 8;
                std::copy(arr, arr + current_byte_capacity, new_arr);
                delete[] arr;
            }

            arr = new_arr;
            capacity_ = new_capacity;
        }
    }

    void resize(std::size_t new_size, bool value = false) {
        reserve(new_size);
        for (std::size_t i = size_; i < new_size; ++i) {
            (*this)[i] = value;
        }
        size_ = new_size;
    }

    void clear() {
        delete[] arr;
        arr = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    Vector() = default;

    ~Vector() {
        delete[] arr;
    }
};

void test_push_back_and_size() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    assert(vec.size() == 3);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 3);
}

void test_capacity_and_reserve() {
    Vector<int> vec;
    vec.reserve(10);
    assert(vec.capacity() == 10);
    vec.push_back(1);
    vec.push_back(2);
    assert(vec.capacity() == 10);
}

void test_resize_without_value() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.resize(5);
    for (auto& i : vec) {
        std::cout << i << std::endl;
    }
    assert(vec.size() == 5);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 0);
}

void test_resize_with_value() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.resize(5, 42);
    assert(vec.size() == 5);
    assert(vec[2] == 42);
    assert(vec[3] == 42);
    assert(vec[4] == 42);
}

void test_pop_back() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.pop_back();
    assert(vec.size() == 1);
    assert(vec[0] == 1);
}

void test_insert() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(3);
    auto it = vec.insert(vec.cbegin() + 1, 2);
    assert(vec.size() == 3);
    assert(vec[1] == 2);
    assert(*it == 2);
}

void test_erase_one() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    auto it = vec.erase(vec.cbegin() + 1);
    assert(vec.size() == 2);
    assert(vec[1] == 3);
    assert(*it == 3);
}

void test_erase_range() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    auto it = vec.erase(vec.cbegin() + 1, vec.cbegin() + 3);
    assert(vec.size() == 2);
    assert(vec[1] == 4);
    assert(*it == 4);
}

void test_front_and_back_single_element() {
    Vector<int> vec;
    vec.push_back(42);
    assert(vec.front() == 42);
    assert(vec.back() == 42);
}

void test_clear_and_empty() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.clear();
    assert(vec.empty());
    assert(vec.size() == 0);
}

void test_shrink_to_fit() {
    Vector<int> vec;
    vec.reserve(100);
    vec.push_back(1);
    vec.push_back(2);
    vec.shrink_to_fit();
    assert(vec.capacity() == 2);
}

void test_at_out_of_range() {
    Vector<int> vec;
    vec.push_back(1);
    try {
        vec.at(2);
        assert(false);
    }
    catch (const std::out_of_range&) {
        assert(true);
    }
}

void test_reserve_smaller_capacity() {
    Vector<int> vec;
    vec.reserve(10);
    vec.reserve(5);
    assert(vec.capacity() == 10); 
}

void test_iterators() {
    Vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    int sum = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        sum += *it;
    }
    assert(sum == 6);
}

int main() {
    test_push_back_and_size();
    test_capacity_and_reserve();
    test_resize_without_value();
    test_resize_with_value();
    test_pop_back();
    test_insert();
    test_erase_one();
    test_erase_range();
    test_front_and_back_single_element();
    test_clear_and_empty();
    test_shrink_to_fit();
    test_at_out_of_range();
    test_reserve_smaller_capacity();
    test_iterators();
    return 0;
}
