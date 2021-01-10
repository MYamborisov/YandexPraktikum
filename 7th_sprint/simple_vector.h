#pragma once

#include <initializer_list>
#include "array_ptr.h"
#include <algorithm>
#include <utility>

struct ReserveProxyObj {
    explicit ReserveProxyObj(std::size_t size) : capacity(size) { }
    std::size_t capacity;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
    
    SimpleVector() noexcept = default;
    
    SimpleVector(ReserveProxyObj obj) {
        Reserve(obj.capacity);
    }
    
    SimpleVector(size_t size, const Type& value = Type{}) : size_(size), capacity_(size), items(size_) {
        std::fill(begin(), end(), value);
    }
    
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(size_), items(size_) {
        std::move(init.begin(), init.end(), begin());
    }
    
    SimpleVector(const SimpleVector& other) : size_(other.GetSize()), capacity_(other.GetCapacity()), items(other.GetSize()) {
        std::copy(other.begin(), other.end(), begin());
    }
    
    SimpleVector(SimpleVector&& other) {
        swap(other);
    }
    
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (rhs == *this) {
            return *this;
        }
        SimpleVector copy(rhs);
        swap(copy);
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& other) {
        size_ = std::exchange(other.size_, 0);
		capacity_ = std::exchange(other.capacity_, 0);
        std::move(other.begin(), other.end(), items.Get());
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        Insert(end(), item);
    }
    
    void PushBack(Type&& value) {
        Insert(end(), std::move(value));
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
		return InsertInVector(pos, value);
	}
    
	Iterator Insert(ConstIterator pos, Type&& value) {
		return InsertInVector(pos, std::move(value));
	}
    
    Iterator InsertInVector(ConstIterator pos, Type&& value) {
        int dist = pos - begin();
        if (size_ == capacity_) {
            size_t new_capacity = std::max(1, (int)capacity_ * 2);
			ArrayPtr<Type> tmp(new_capacity);
			std::move(begin(), end(), tmp.Get());
			items.swap(tmp);
			capacity_ = new_capacity;
        }
        std::move_backward(begin() + dist, end(), end() + 1);
        *(begin() + dist) = std::move(value);
        ++size_;
        return begin() + dist;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
        --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        int dist = pos - begin();
        std::move(begin() + dist + 1, end(), begin() + dist);
        --size_;
        return begin() + dist;
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> tmp(new_capacity);
			std::move(begin(), end(), tmp.Get());
			items.swap(tmp);
			capacity_ = new_capacity;
        }
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items.swap(other.items);
    }
    
    size_t GetSize() const noexcept {
        return size_;
    }
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
    bool IsEmpty() const noexcept {
        if(size_ == 0) {
            return true;
        }
        return false;
    }
    Type& operator[](size_t index) noexcept {
        return items[index];
    }
    const Type& operator[](size_t index) const noexcept {
        return items[index];
    }
    Type& At(size_t index) {
        if(index >= size_) {
            throw std::out_of_range("Index over size");
        }
        return items[index];
    }
    const Type& At(size_t index) const {
        if(index >= size_) {
            throw std::out_of_range("Index over size");
        }
        return items[index];
    }
    void Clear() noexcept {
        size_ = 0;
    }
    void Resize(size_t new_size) {
        if (new_size > capacity_) {
			size_t new_capacity = std::max(new_size, capacity_ * 2);
			ArrayPtr<Type> tmp(new_capacity);
			std::copy(begin(), end(), tmp.Get());
			items.swap(tmp);
			std::fill(end(), begin() + (new_capacity - capacity_), Type{});
			capacity_ = new_capacity;
		}
		else if (new_size > size_) {
			std::fill(end(), begin() + new_size, Type{});
		}
		size_ = new_size; 
    }
    Iterator begin() noexcept {
        return items.Get();
    }
    Iterator end() noexcept {
        return items.Get() + size_;
    }
    ConstIterator begin() const noexcept {
        return items.Get();
    }
    ConstIterator end() const noexcept {
        return items.Get() + size_;
    }
    ConstIterator cbegin() const noexcept {
        return items.Get();
    }
    ConstIterator cend() const noexcept {
        return items.Get() + size_;
    }
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> items;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize() and std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}