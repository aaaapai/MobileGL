#pragma once
#include <Includes.h>

namespace MobileGL {
    template <typename IndexType>
    class IndexGenerator {
    public:
        explicit IndexGenerator(SizeT initial_capacity = 1024, IndexType first_index = 0) : next_index_(first_index) {
            const SizeT words_needed = (initial_capacity + 63) / 64;
            is_valid_.resize(words_needed, ~0ull);
            freed_indices_.reserve(initial_capacity);
        }

        void Generate(SizeT n, IndexType* indices) {
            if (n == 0) return;

            SizeT from_freed = std::min(n, freed_indices_.size());
            if (from_freed > 0) {
                std::copy_n(freed_indices_.end() - from_freed, from_freed, indices);
                freed_indices_.resize(freed_indices_.size() - from_freed);

                for (SizeT i = 0; i < from_freed; ++i) {
                    SetValid(indices[i], true);
                }
            }

            SizeT need_new = n - from_freed;
            if (need_new > 0) {
                SizeT required_index = next_index_ + need_new;
                SizeT required_words = (required_index + 63) / 64;
                if (required_words > is_valid_.size()) {
                    is_valid_.resize(required_words, ~0ull);
                }

                for (SizeT i = 0; i < need_new; ++i) {
                    indices[from_freed + i] = next_index_++;
                }
            }
        }

        void Delete(IndexType index) {
            SetValid(index, false);
            freed_indices_.push_back(index);
            if (freed_indices_.size() > 1024 && freed_indices_.size() > next_index_ / 2) {
                CompactFreeList();
            }
        }

        Bool IsValid(IndexType index) const {
            SizeT word = index >> 6;
            SizeT bit = index & 0x3F;
            return word < is_valid_.size() && (is_valid_[word] & (1ull << bit));
        }

        SizeT FreeListSize() const { return freed_indices_.size(); }
        SizeT ActiveCount() const { return next_index_ - freed_indices_.size(); }

    private:
        inline void SetValid(IndexType index, Bool valid) {
            SizeT word = index >> 6;
            uint64_t mask = 1ull << (index & 0x3F);

            if (word >= is_valid_.size()) return;

            if (valid) {
                is_valid_[word] |= mask;
            } else {
                is_valid_[word] &= ~mask;
            }
        }

        void CompactFreeList() {
            std::sort(freed_indices_.begin(), freed_indices_.end());
            auto last = std::unique(freed_indices_.begin(), freed_indices_.end());
            freed_indices_.erase(last, freed_indices_.end());
        }

    private:
        IndexType next_index_ = 0;
        std::vector<IndexType> freed_indices_;
        std::vector<Uint64> is_valid_;
    };
} // namespace MobileGL