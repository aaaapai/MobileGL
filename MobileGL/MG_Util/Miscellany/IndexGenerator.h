#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>

namespace MobileGL {
    template <typename IndexType>
    class IndexGenerator {
    public:
        explicit IndexGenerator(size_t initial_capacity = 1024)
            : next_index_(0)
        {
            is_valid_.reserve(initial_capacity);
        }

        void Generate(size_t n, IndexType* indices) {
            if (n == 0) return;

            size_t from_freed = std::min(n, freed_indices_.size());
            for (size_t i = 0; i < from_freed; ++i) {
                indices[i] = freed_indices_.back();
                freed_indices_.pop_back();
                is_valid_[indices[i]] = 1;
            }

            size_t need_new = n - from_freed;
            if (need_new > 0) {
                size_t required = next_index_ + need_new;
                if (required > is_valid_.size()) {
                    size_t new_cap = std::max(required, is_valid_.size() * 2);
                    is_valid_.resize(new_cap, 0);
                }
                for (size_t i = 0; i < need_new; ++i) {
                    indices[from_freed + i] = next_index_;
                    is_valid_[next_index_] = 1;
                    ++next_index_;
                }
            }
        }

        void Delete(IndexType index) {
            if (index < is_valid_.size() && is_valid_[index]) {
                is_valid_[index] = 0;
                freed_indices_.push_back(index);
            }
        }

        bool IsValid(IndexType index) const {
            return index < is_valid_.size() && is_valid_[index];
        }

    private:
        IndexType                next_index_ = 0;
        std::vector<IndexType>   freed_indices_;
        std::vector<UInt8>     is_valid_;
    };

}
