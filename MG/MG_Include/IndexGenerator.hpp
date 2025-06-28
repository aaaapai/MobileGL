//
// Created by yello on 2025-06-27.
//

#ifndef INDEXGENERATOR_H
#define INDEXGENERATOR_H

#include <vector>
#include <cstddef> // For size_t
#include <algorithm> // For std::min
#include <memory>

template <typename IndexType>
class IndexGenerator {
public:
    // 构造函数
    IndexGenerator() : next_index_(1) {}
    explicit IndexGenerator(IndexType n) : next_index_(1) {
        is_valid_.reserve(n);
    }

    // 生成n个新正整数
    void Generate(size_t n, IndexType *indices) {
        if (n <= 0) {
            return;
        }

        // 优先给出被删除的索引
        size_t count_from_freed = std::min(n, freed_indices_.size());
        for (size_t i = 0; i < count_from_freed; ++i) {
            indices[i] = freed_indices_.back();
            freed_indices_.pop_back();
            // 标记为有效，确保is_valid_的大小足够
            if (indices[i] >= is_valid_.size()) {
                is_valid_.resize(static_cast<size_t>(indices[i]) + 1, false); // 0代表无效，1代表有效
            }
            is_valid_[indices[i]] = true; // 标记为有效
        }

        // 如果还需要更多的索引，则生成新的索引
        for (size_t i = count_from_freed; i < n; ++i) {
            indices[i] = next_index_++;
            // 确保is_valid_的大小足够
            if (indices[i] >= is_valid_.size()) {
                is_valid_.resize(static_cast<size_t>(indices[i]) + 1, false);
            }
            is_valid_[indices[i]] = true; // 标记为有效
        }
    }

    // 删除一个已经给出的正整数
    void Delete(IndexType index) {
        // 检查索引是否有效且未被删除
        // 这里的检查是为了避免重复删除或删除未生成的索引
        if (index < is_valid_.size() && is_valid_[index] == true) {
            is_valid_[index] = false; // 标记为无效
            freed_indices_.push_back(index); // 添加到已删除列表中
        }
    }

    // 检查一个正整数是否之前已经被给出，并且没有被删除
    bool IsValid(IndexType index) const {
        // 如果索引超出is_valid_的范围，或者对应的位置是0，则认为无效
        return index < is_valid_.size() && is_valid_[index] == true;
    }

private:
    IndexType next_index_; // 下一个将要生成的正整数
    std::vector<IndexType> freed_indices_; // 存储被删除的索引
    std::vector<bool> is_valid_; // 标记索引是否有效。0代表无效，1代表有效。
};

#endif //INDEXGENERATOR_H
