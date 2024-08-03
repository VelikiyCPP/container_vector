#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

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
        using pointer_type = typename std::conditional_t<is_const, const T*, T*>;
        using reference_type = typename std::conditional_t<is_const, const T&, T&>;
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        pointer_type ptr_;
        pointer_type begin_;
        pointer_type end_;

        base_iterator(pointer_type ptr, pointer_type begin, pointer_type end)
            : ptr_(ptr), begin_(begin), end_(end) {}
        base_iterator(const base_iterator& other) = default;
        base_iterator& operator=(const base_iterator& other) = default;

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

        base_iterator operator++(int) {
            base_iterator copy = *this;
            ++ptr_;
            check_range();
            return copy;
        }

        base_iterator& operator--() {
            --ptr_;
            check_range();
            return *this;
        }

        base_iterator operator--(int) {
            base_iterator copy = *this;
            --ptr_;
            check_range();
            return copy;
        }

        base_iterator operator+(const difference_type value) const {
            base_iterator temp = *this;
            temp.ptr_ += value;
            temp.check_range();
            return temp;
        }

        base_iterator operator-(const difference_type value) const {
            base_iterator temp = *this;
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

    private:
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

    Vector(size_type n) : capacity_(n), size_(n), data_(reinterpret_cast<T*>(::new char[n * sizeof(T)])) {
        std::uninitialized_default_construct_n(data_, n);
    }

    Vector(size_type n, const_reference value)
        : capacity_(n), size_(n), data_(reinterpret_cast<T*>(::new char[n * sizeof(T)])) {
        std::uninitialized_fill_n(data_, n, value);
    }

    Vector(const std::initializer_list<T>& list)
        : capacity_(list.size()), size_(list.size()), data_(reinterpret_cast<T*>(::new char[capacity_ * sizeof(T)])) {
        std::uninitialized_copy(list.begin(), list.end(), data_);
    }

    void reserve(const size_type new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        value_type* new_arr;
        try {
            new_arr = static_cast<T*>(operator ::new[](new_capacity * sizeof(T)));
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
                std::uninitialized_default_construct_n(data_ + size_, count - size_);
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
    const_iterator cbegin() const noexcept { iterator(data_, data_, data_ + size_); }

    iterator end() noexcept { return iterator(data_ + size_, data_, data_ + size_); }
    const_iterator cend() const noexcept { return iterator(data_ + size_, data_, data_ + size_); }

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

        if (ptr + 1 < data_ + size_) {
            std::uninitialized_copy(ptr + 1, data_ + size_, ptr);
            std::destroy_at(data_ + size_ - 1);
        }

        --size_;

        return iterator(ptr, data_, data_ + size_);
    }

    iterator erase(iterator first, iterator last) {
        if (first.ptr_ < data_ || first.ptr_ >= data_ + size_ || last.ptr_ < data_ || last.ptr_ > data_ + size_ || first.ptr_ > last.ptr_) {
            throw std::out_of_range("Iterator out of range");
        }

        pointer ptr_first = first.ptr_;
        pointer ptr_last = last.ptr_;

        std::uninitialized_copy(ptr_last, data_ + size_, ptr_first);
        std::destroy(ptr_first + (data_ + size_ - ptr_last), data_ + size_);

        size_ -= (ptr_last - ptr_first);

        return iterator(ptr_first, data_, data_ + size_);
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

public:
    Bit_reference operator[](std::size_t index) {
        std::uint8_t pos = index % 8;
        std::uint8_t* ptr = arr + index / 8;

        return Bit_reference(ptr, pos);
    }
};

#include <vector>

int main() {
    Vector<int> vec;
    std::cout << "Initial vector size: " << vec.size() << std::endl;

    vec.resize(5, 10);
    std::cout << "Vector size after resize(5, 10): " << vec.size() << std::endl;
    std::cout << "Vector capacity after resize(5, 10): " << vec.capacity() << std::endl;

    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "Element at index " << i << ": " << vec[i] << std::endl;
    }

    vec.push_back(20);
    std::cout << "Vector size after push_back(20): " << vec.size() << std::endl;
    std::cout << "Element at index 5 (last element): " << vec[5] << std::endl;

    vec.pop_back();
    std::cout << "Vector size after pop_back(): " << vec.size() << std::endl;

    auto it = vec.begin() + 2;
    vec.insert(it, 30);
    std::cout << "Vector size after insert(30): " << vec.size() << std::endl;
    std::cout << "Element at index 2 (after insert): " << vec[2] << std::endl;

    vec.erase(it);
    std::cout << "Vector size after erase(): " << vec.size() << std::endl;
    std::cout << "Element at index 2 (after erase): " << vec[2] << std::endl;

    vec.resize(7, 40);
    std::cout << "Vector size after resize(7, 40): " << vec.size() << std::endl;
    std::cout << "Element at index 5 (after resize): " << vec[5] << std::endl;

    vec.resize(3);
    std::cout << "Vector size after resize(3): " << vec.size() << std::endl;

    vec.clear();
    std::cout << "Vector size after clear(): " << vec.size() << std::endl;

    try {
        vec.at(100);
    }
    catch (const std::out_of_range& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }

    Vector<int> vec_init = { 1, 2, 3, 4, 5 };
    std::cout << "Vector size with initializer_list: " << vec_init.size() << std::endl;
    for (const auto& i : vec_init) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    std::cout << "Vector elements in reverse order: ";
    for (auto it = vec_init.rbegin(); it != vec_init.rend() + 1; ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;

    vec_init.shrink_to_fit();
    std::cout << "Vector capacity after shrink_to_fit: " << vec_init.capacity() << std::endl;

    return 0;
}
