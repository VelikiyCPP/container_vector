#pragma once

#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <type_traits>

namespace np {
    template <typename T, typename Allocator = std::allocator<T>>
    class vector {
    public:

        // Allocator
        using allocator_type = Allocator;
        using allocator_traits = std::allocator_traits<allocator_type>;

        // Type
        using value_type = T;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = typename std::allocator_traits<allocator_type>::pointer;
        using const_pointer = const pointer;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;

    private:

        size_type capacity_ = 0;
        size_type size_ = 0;

        pointer data_ = nullptr;

        allocator_type allocator_;

        template <bool is_const>
        class base_iterator {
        public:
            using pointer_type = std::conditional_t<is_const, const_pointer, pointer>;
            using reference_type = std::conditional_t<is_const, const_reference, reference>;
            using difference_type = std::ptrdiff_t;

            pointer_type ptr_ = nullptr;
            pointer_type begin_ = nullptr;
            pointer_type end_ = nullptr;

            /***************************/
            base_iterator() noexcept = default;

            explicit base_iterator(pointer_type ptr, pointer_type begin = nullptr, pointer_type end = nullptr) noexcept : ptr_(ptr), begin_(begin), end_(end) {}

            base_iterator(const base_iterator& other) noexcept : ptr_(other.ptr_), begin_(other.begin_), end_(other.end_) {}

            base_iterator(base_iterator&& other) noexcept = default; // *

            base_iterator& operator=(const base_iterator& other) noexcept = default;

            base_iterator& operator=(base_iterator&& other) noexcept = default; // *
            /***************************/


            /***************************/
            reference_type operator*() const {
                return *ptr_;
            }

            pointer_type operator->() const {
                return ptr_;
            }
            /***************************/



            /***************************/
            base_iterator& operator++() {
                ++ptr_;
                return *this;
            }

            base_iterator& operator--() {
                --ptr_;
                return *this;
            }

            base_iterator operator++(int) {
                base_iterator temp = *this;
                ++ptr_;
                return temp;
            }

            base_iterator operator--(int) {
                base_iterator temp = *this;
                --ptr_;
                return temp;
            }
            /***************************/



            /***************************/
            base_iterator operator+(difference_type value) const {
                return base_iterator(ptr_ + value, begin_, end_);
            }

            base_iterator operator-(difference_type value) const {
                return base_iterator(ptr_ - value, begin_, end_);
            }

            difference_type operator-(const base_iterator& other) const {
                return ptr_ - other.ptr_;
            }

            base_iterator& operator+=(difference_type value) {
                ptr_ += value;
                return *this;
            }

            base_iterator& operator-=(difference_type value) {
                ptr_ -= value;
                return *this;
            }
            /***************************/



            /***************************/
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
            /***************************/



            /***************************/
            explicit operator base_iterator<true>() const {
                return base_iterator<true>(ptr_, begin_, end_);
            }

            explicit operator base_iterator<false>() const {
                return base_iterator<false>(ptr_, begin_, end_);
            }
            /***************************/
        };
    public:
        using iterator = base_iterator<false>;
        using const_iterator = base_iterator<true>;

        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    public:
        vector() = default;

        explicit vector(const size_type n) : capacity_(n), size_(n), data_(allocator_traits::allocate(allocator_, n)) {
            std::uninitialized_default_construct_n(data_, n);
        }

        vector(const size_type n, const_reference value) : capacity_(n), size_(n), data_(allocator_traits::allocate(allocator_, n)) {
            std::uninitialized_fill_n(data_, n, value);
        }

        vector(const std::initializer_list<T>& list) : capacity_(list.size()), size_(list.size()), data_(allocator_traits::allocate(allocator_, capacity_)) {
            std::uninitialized_copy(list.begin(), list.end(), data_);
        }

        template <class InputIt>
        vector(InputIt first, InputIt last) {
            if (first == last) {
                return;
            }

            size_type distance = std::distance(first, last);
            reserve(distance);

            for (; first != last; ++first) {
                push_back(*first);
            }
        }

        vector (const vector& other) {
            Allocator new_allocator = allocator_traits::select_on_container_copy_construction()
                ? other.allocator_
                : allocator_;

            pointer new_arr = allocator_traits::allocate(new_allocator, other.capacity_);

            size_type index = 0;
            try {
                for (; index < other.size_; ++index) {
                    allocator_traits::construct(new_allocator, new_arr + index, other[index]);
                }
            } catch (...) {
                for (size_type i = 0; i < index; ++i) {
                    allocator_traits::destroy(new_allocator, new_arr + i);
                }

                allocator_traits::deallocate(new_allocator, new_arr, other.capacity_);
                throw;
            }

            if (data_ != nullptr) {
                for (size_type i = 0; i < other.size_; ++i) {
                    allocator_traits::destroy(allocator_, data_ + i);
                }

                allocator_traits::deallocate(allocator_, data_, capacity_);
            }

            data_ = new_arr;
            size_ = other.size_;
            capacity_ = other.capacity_;
            allocator_ = new_allocator;
        }

        vector& operator=(const vector& other) {
            Allocator new_allocator = allocator_traits::propagate_on_container_copy_assignment::value
                ? other.allocator_
                : allocator_;

            pointer new_arr = allocator_traits::allocate(new_allocator, other.capacity_);

            size_type index = 0;
            try {
                for (; index < other.size_; ++index) {
                    allocator_traits::construct(new_allocator, new_arr + index, other[index]);
                }
            } catch (...) {
                for (size_type i = 0; i < index; ++i) {
                    allocator_traits::destroy(new_allocator, new_arr + i);
                }
                allocator_traits::deallocate(new_allocator, new_arr, other.capacity_);
                throw;
            }

            if (data_ != nullptr) {
                for (size_type i = 0; i < size_; ++i) {
                    allocator_traits::destroy(allocator_, data_ + i);
                }

                allocator_traits::deallocate(allocator_, data_, capacity_);
            }

            data_ = new_arr;
            size_ = other.size_;
            capacity_ = other.capacity_;

            return *this;
        }

        void reserve(const size_type new_capacity) {
            if (new_capacity <= capacity_) {
                return;
            }

            pointer new_arr = allocator_traits::allocate(allocator_, new_capacity);

            size_type index = 0;
            try {
                for (; index < size_; ++index) {
                    allocator_traits::construct(allocator_, new_arr + index, data_[index]);
                }
            }
            catch (...) {
                for (size_type i = 0; i < index; ++i) {
                    allocator_traits::destroy(allocator_, new_arr + i);
                }

                allocator_traits::deallocate(allocator_, new_arr, capacity_);
                throw;
            }

            for (size_type i = 0; i < size_; ++i) {
                allocator_traits::destroy(allocator_, data_ + i);
            }

            allocator_traits::deallocate(allocator_, data_, new_capacity);

            data_ = new_arr;
            capacity_ = new_capacity;
        }

        void push_back(const_reference element) {
            if (size_ == capacity_) {
                reserve(capacity_ ? capacity_ * 2 : 1);
            }

            typename std::allocator_traits<Allocator>::pointer ptr = allocator_traits::allocate(allocator_, 1);
            try {
                std::allocator_traits<Allocator>::construct(allocator_, ptr, element);

                data_[size_++] = *ptr;
            }
            catch (...) {
                allocator_traits::deallocate(allocator_, ptr, 1);
                throw;
            }
        }

        void pop_back() {
            allocator_traits::destroy(allocator_, data_ + --size_);
        }

        void clear() {
            for (; size_ > 0; --size_) {
                allocator_traits::destroy(allocator_, data_ + size_ - 1);
            }
        }

        void shrink_to_fit() {
            if (size_ < capacity_) {
                pointer new_arr = allocator_traits::allocate(allocator_, size_);

                size_type index = 0;
                try {
                    for (; index < size_; ++index) {
                        allocator_traits::construct(allocator_, new_arr + index, data_[index]);
                    }
                } catch (...) {
                    for (; index > 0; --index) {
                        allocator_traits::destroy(allocator_, new_arr + index - 1);
                    }

                    allocator_traits::deallocate(allocator_, new_arr, size_);
                    throw;
                }

                for (size_type i = size_; i < capacity_; ++i) {
                    allocator_traits::destroy(allocator_, data_ + i);
                }

                allocator_traits::deallocate(allocator_, data_, capacity_);

                data_ = new_arr;
                capacity_ = size_;
            }
        }

       void resize(const size_type count) {
            if (count > capacity_) {
                reserve(count);
            }

            if (count == 0) {
                clear();
            }
            else {
                if (count > size_) {
                    try {
                        for (size_type i = size_; i < count; ++i) {
                            allocator_traits::construct(allocator_, data_ + i, value_type());
                        }
                    } catch (...) {
                        throw;
                    }
                }
                else {
                    for (size_type i = count; i < size_; ++i) {
                        allocator_traits::destroy(allocator_, data_ + i);
                    }
                }
            }

            size_ = count;
        }

        void resize(const size_type count, const_reference value) {
            if (count > capacity_) {
                reserve(count);
            }

            if (count == 0) {
                clear();
            }
            else {
                if (count > size_) {
                    try {
                        for (size_type i = size_; i < count; ++i) {
                            allocator_traits::construct(allocator_, data_ + i, value);
                        }
                    } catch (...) {
                        throw;
                    }
                }
                else {
                    for (size_type i = count; i < size_; ++i) {
                        allocator_traits::destroy(allocator_, data_ + i);
                    }
                }
            }

            size_ = count;
        }

        [[nodiscard]] size_type size() const noexcept { return size_; }
        [[nodiscard]] size_type capacity() const noexcept { return capacity_; }

        iterator insert(const_iterator pos, const_reference value) {
            difference_type index = pos.ptr_ - data_;
            if (size_ > capacity_ - 1) {
                reserve(capacity_ ? capacity_ * 2 : 1);
            }

            const_pointer ptr = data_ + index;

            if (ptr < data_ + size_) {
                std::uninitialized_copy(ptr, data_ + size_, ptr + 1);
                allocator_traits::destroy(allocator_, ptr);
            }

            allocator_traits::construct(allocator_, ptr, value);
            ++size_;

            return iterator(data_ + index, data_, data_ + size_);
        }

        iterator erase(iterator pos) {
            if (pos.ptr_ < data_ || pos.ptr_ >= data_ + size_) {
                throw std::out_of_range("Iterator out of range");
            }

            pointer ptr = pos.ptr_;

            for (pointer p = ptr + 1; p != data_ + size_; ++p) {
                *(p - 1) = *p;
            }

            allocator_traits::destroy(allocator_, data_ + size_ - 1);

            --size_;

            return iterator(ptr, data_, data_ + size_);
        }

        iterator erase(iterator first, iterator last) {
            if (first.ptr_ < data_ || first.ptr_ >= data_ + size_ || last.ptr_ < data_ || last.ptr_ > data_ + size_ || first.ptr_ > last.ptr_) {
                throw std::out_of_range("Iterator out of range");
            }

            pointer ptr_first = first.ptr_;
            pointer ptr_last = last.ptr_;

            for (pointer p = ptr_last; p != data_ + size_; ++p) {
                *(p - std::distance(ptr_first, ptr_last)) = *p;
            }

            for (size_type i = 0; i < std::distance(ptr_first, ptr_last); ++i) {
                allocator_traits::destroy(allocator_, data_ + size_ - i);
            }

            size_ -= std::distance(ptr_first, ptr_last);

            return iterator(ptr_first, data_, data_ + size_);
        }

        iterator erase(const_iterator pos) {
            return erase(static_cast<iterator>(pos));
        }

        iterator erase(const_iterator first, const_iterator last) {
            return erase(static_cast<iterator>(first), static_cast<iterator>(last));
        }

        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

        iterator begin() noexcept { return iterator(data_); }
        const_iterator cbegin() const noexcept { return const_iterator(data_); }

        iterator end() noexcept { return iterator(data_ + size_); }
        const_iterator cend() const noexcept { return const_iterator(data_ + size_); }

        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

        const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
        const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

        reference front() { return *data_; }
        const_reference front() const { return *data_; }

        reference back() { return data_[size_ - 1]; }
        const_reference back() const { return data_[size_ - 1]; }

        reference operator[](const size_type index) {
            return data_[index];
        }

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

        ~vector() {
            if (data_) {
                for (size_type i = 0; i < size_; ++i) {
                    allocator_traits::destroy(allocator_, data_ + i);
                }

                allocator_traits::deallocate(allocator_, data_, capacity_);
            }
        }
    };
}

template<>
    class vector<bool> {

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

        uint8_t* arr_;
        std::size_t size_;
        std::size_t capacity_;

    public:
        Bit_reference operator[](std::size_t index) {
            std::uint8_t pos = index % 8;
            std::uint8_t* ptr = arr_ + index / 8;

            return Bit_reference(ptr, pos);
        }

        vector(const std::initializer_list<bool>& list) {
            size_ = list.size();
            capacity_ = (size_ + 7) / 8;
            arr_ = new uint8_t[capacity_]();

            std::size_t i = 0;
            for (bool value : list) {
                if (value) {
                    arr_[i / 8] |= (static_cast<uint8_t>(1) << (i % 8));
                }
                ++i;
            }
        }

        vector& operator=(const vector& other) {
            if (this != &other) {
                size_ = other.size_;
                capacity_ = other.capacity_;
                arr_ = new uint8_t[capacity_];
                std::copy(other.arr_, other.arr_ + capacity_, arr_);
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
            if (arr_ != nullptr) {
                std::size_t pos = size_ % 8;
                arr_[size_ / 8] &= ~(static_cast<uint8_t>(1) << (7 - pos));
            }
        }

        void push_back(bool value) {
            if (size_ == capacity_ * 8) {
                reserve(capacity_ == 0 ? 1 : capacity_ * 2);
            }
            if (value) {
                arr_[size_ / 8] |= (static_cast<uint8_t>(1) << (size_ % 8));
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

                if (arr_ != nullptr) {
                    std::size_t current_byte_capacity = (capacity_ + 7) / 8;
                    std::copy(arr_, arr_ + std::min(current_byte_capacity, new_byte_capacity), new_arr);
                    delete[] arr_;
                }

                arr_ = new_arr;
                capacity_ = new_capacity;
                size_ = new_size;
            }
            else {
                std::size_t new_byte_capacity = (new_capacity + 7) / 8;
                uint8_t* new_arr = new uint8_t[new_byte_capacity]();

                if (arr_ != nullptr) {
                    std::size_t current_byte_capacity = (capacity_ + 7) / 8;
                    std::copy(arr_, arr_ + current_byte_capacity, new_arr);
                    delete[] arr_;
                }

                arr_ = new_arr;
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
            delete[] arr_;
            arr_ = nullptr;
            size_ = 0;
            capacity_ = 0;
        }

        vector() = default;

        ~vector() {
            delete[] arr_;
        }
    };
