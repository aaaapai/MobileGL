#pragma once

namespace MobileGL {
    template <typename IndexType>
    class IndexGenerator {
    public:
        IndexGenerator() : next_index_(1) {}
        explicit IndexGenerator(IndexType n) : next_index_(1) {
            is_valid_.reserve(n);
        }

        void Generate(size_t n, IndexType* indices) {
            if (n <= 0) {
                return;
            }

            size_t count_from_freed = std::min(n, freed_indices_.size());
            for (size_t i = 0; i < count_from_freed; ++i) {
                indices[i] = freed_indices_.back();
                freed_indices_.pop_back();
                if (indices[i] >= is_valid_.size()) {
                    is_valid_.resize(static_cast<size_t>(indices[i]) + 1, false);
                }
                is_valid_[indices[i]] = true;
            }

            for (size_t i = count_from_freed; i < n; ++i) {
                indices[i] = next_index_++;
                if (indices[i] >= is_valid_.size()) {
                    is_valid_.resize(static_cast<size_t>(indices[i]) + 1, false);
                }
                is_valid_[indices[i]] = true;
            }
        }

        void Delete(IndexType index) {
            if (index < is_valid_.size() && is_valid_[index] == true) {
                is_valid_[index] = false;
                freed_indices_.push_back(index);
            }
        }

        bool IsValid(IndexType index) const {
            return index < is_valid_.size() && is_valid_[index] == true;
        }

    private:
        IndexType next_index_;
        std::vector<IndexType> freed_indices_;
        std::vector<bool> is_valid_;
    };
}