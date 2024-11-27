#include <cassert>
#include <iostream>

#include "containers/vector/vector.hpp"

void test_push_back_and_size() {
    np::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    assert(vec.size() == 3);
    assert(vec[0] == 1);
    assert(vec[1] == 2);
    assert(vec[2] == 3);
}

void test_capacity_and_reserve() {
    np::vector<int> vec;
    vec.reserve(10);
    assert(vec.capacity() == 10);
    vec.push_back(1);
    vec.push_back(2);
    assert(vec.capacity() == 10);
}

void test_resize_without_value() {
    np::vector<int> vec;
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
    np::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.resize(5, 42);
    assert(vec.size() == 5);
    assert(vec[2] == 42);
    assert(vec[3] == 42);
    assert(vec[4] == 42);
}

void test_pop_back() {
    np::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.pop_back();
    assert(vec.size() == 1);
    assert(vec[0] == 1);
}

void test_insert() {
    np::vector<int> vec;
    vec.push_back(1);
    vec.push_back(3);
    auto it = vec.insert(vec.cbegin() + 1, 2);
    assert(vec.size() == 3);
    assert(vec[1] == 2);
    assert(*it == 2);
}

void test_erase_one() {
    np::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    auto it = vec.erase(vec.cbegin() + 1);
    assert(vec.size() == 2);
    assert(vec[1] == 3);
    assert(*it == 3);
}

void test_erase_range() {
    np::vector<int> vec;
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
    np::vector<int> vec;
    vec.push_back(42);
    assert(vec.front() == 42);
    assert(vec.back() == 42);
}

void test_clear_and_empty() {
    np::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.clear();
    assert(vec.empty());
    assert(vec.size() == 0);
}

void test_shrink_to_fit() {
    np::vector<int> vec;
    vec.reserve(100);
    vec.push_back(1);
    vec.push_back(2);
    vec.shrink_to_fit();
    assert(vec.capacity() == 2);
}

void test_at_out_of_range() {
    np::vector<int> vec;
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
    np::vector<int> vec;
    vec.reserve(10);
    vec.reserve(5);
    assert(vec.capacity() == 10);
}

void test_iterators() {
    np::vector<int> vec;
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

    np::vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }

    vec.erase(vec.begin() + 1, vec.end() - 1);
    for (auto i : vec) {
        std::cout << i << std::endl;
    }
    return 0;
}