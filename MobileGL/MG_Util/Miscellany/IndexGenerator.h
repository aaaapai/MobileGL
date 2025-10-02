#pragma once
#include <Includes.h>

namespace MobileGL {
    template <typename IndexType>
    class IndexGenerator {
    public:
        static_assert(std::is_integral<IndexType>::value && std::is_unsigned<IndexType>::value,
                      "IndexType must be an unsigned integral type.");

        using SizeT = size_t;

        explicit IndexGenerator(SizeT initial_capacity = 1024, IndexType first_index = 1)
            : m_first_index(first_index), m_next_index(first_index), m_active_count(0) {

            m_free_list.reserve(initial_capacity);

            SizeT initial_bits_size = std::max(static_cast<SizeT>(first_index), initial_capacity);
            m_valid_bits.resize(initial_bits_size, false);
        }

        void Generate(SizeT n, IndexType* indices) noexcept {
            if (n == 0) return;

            const SizeT from_free_list_count = std::min(n, m_free_list.size());
            if (from_free_list_count > 0) {
                const auto free_list_start = m_free_list.end() - from_free_list_count;
                std::copy(free_list_start, m_free_list.end(), indices);
                m_free_list.erase(free_list_start, m_free_list.end());

                for (SizeT i = 0; i < from_free_list_count; ++i) {
                    m_valid_bits[indices[i]] = true;
                }
            }

            const SizeT new_indices_count = n - from_free_list_count;
            if (new_indices_count > 0) {
                const IndexType required_size = m_next_index + new_indices_count;
                if (required_size > m_valid_bits.size()) {
                    m_valid_bits.resize(std::max(required_size, static_cast<IndexType>(m_valid_bits.size() * 1.5)));
                }

                IndexType* new_indices_ptr = indices + from_free_list_count;
                for (SizeT i = 0; i < new_indices_count; ++i) {
                    const IndexType new_index = m_next_index++;
                    new_indices_ptr[i] = new_index;
                    m_valid_bits[new_index] = true;
                }
            }

            m_active_count += n;
        }

        void Delete(IndexType index) noexcept {
            if (!IsValid(index)) {
                return;
            }

            m_valid_bits[index] = false;
            m_free_list.push_back(index);
            m_active_count--;
        }

        bool IsValid(IndexType index) const noexcept { return index < m_valid_bits.size() && m_valid_bits[index]; }

        SizeT FreeListSize() const noexcept { return m_free_list.size(); }

        SizeT ActiveCount() const noexcept { return m_active_count; }

    private:
        IndexType m_first_index;
        IndexType m_next_index;
        SizeT m_active_count;
        std::vector<IndexType> m_free_list;
        std::vector<bool> m_valid_bits;
    };

} // namespace MobileGL