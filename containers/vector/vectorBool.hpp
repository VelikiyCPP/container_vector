#pragma once

#include "vector.hpp"

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