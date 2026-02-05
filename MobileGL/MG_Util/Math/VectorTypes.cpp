// MobileGL - MobileGL/MG_Util/Math/VectorTypes.cpp
// Copyright (c) 2025-2026 MobileGL-Dev
// Licensed under the GNU Lesser General Public License v3.0:
//   https://www.gnu.org/licenses/gpl-3.0.txt
//   https://www.gnu.org/licenses/lgpl-3.0.txt
// SPDX-License-Identifier: LGPL-3.0-only
// End of Source File Header

#include "VectorTypes.h"

namespace MobileGL {
    void VecRange1D::Add(const Range1D& newRange, Double ratio, SizeT* outMinStart, SizeT* outMaxEnd) {
        if (this->empty()) {
            this->push_back(newRange);
            m_overallMaxEnd = newRange.end;
            if (outMinStart) *outMinStart = this->front().start;
            if (outMaxEnd) *outMaxEnd = m_overallMaxEnd;
            return;
        }

        auto it = std::lower_bound(this->begin(), this->end(), newRange.start,
                                   [](const Range1D& a, SizeT valueStart) { return a.start < valueStart; });
        size_t pos = static_cast<size_t>(std::distance(this->begin(), it));

        auto calc_gap = [](const Range1D& a, const Range1D& b) -> SizeT {
            return (b.start > a.end) ? (b.start - a.end) : 0;
        };

        auto calc_threshold = [ratio](const Range1D& a, const Range1D& b) -> SizeT {
            SizeT minStart = (a.start < b.start) ? a.start : b.start;
            SizeT maxEnd = (a.end > b.end) ? a.end : b.end;
            SizeT span = (maxEnd > minStart) ? (maxEnd - minStart) : 0;
            return static_cast<SizeT>(static_cast<Double>(span) * ratio);
        };

        if (pos >= this->size()) {
            Range1D& last = this->back();
            SizeT gap = calc_gap(last, newRange);
            SizeT threshold = calc_threshold(last, newRange);

            if (gap <= threshold) {
                last.end = std::max(last.end, newRange.end);
                m_overallMaxEnd = std::max(m_overallMaxEnd, last.end);
            } else {
                this->push_back(newRange);
                m_overallMaxEnd = std::max(m_overallMaxEnd, newRange.end);
            }

            if (outMinStart) *outMinStart = this->front().start;
            if (outMaxEnd) *outMaxEnd = m_overallMaxEnd;
            return;
        }

        bool merged = false;
        if (pos > 0) {
            Range1D& prev = (*this)[pos - 1];
            SizeT gapPrev = calc_gap(prev, newRange);
            SizeT thresholdPrev = calc_threshold(prev, newRange);

            if (gapPrev <= thresholdPrev) {
                prev.end = std::max(prev.end, newRange.end);
                m_overallMaxEnd = std::max(m_overallMaxEnd, prev.end);

                size_t writeIdx = pos - 1;
                while (writeIdx + 1 < this->size()) {
                    Range1D& cur = (*this)[writeIdx];
                    Range1D& nxt = (*this)[writeIdx + 1];
                    SizeT gap = calc_gap(cur, nxt);
                    SizeT threshold = calc_threshold(cur, nxt);
                    if (gap <= threshold) {
                        // merge nxt into cur
                        cur.end = std::max(cur.end, nxt.end);
                        this->erase(this->begin() + (writeIdx + 1));
                        m_overallMaxEnd = std::max(m_overallMaxEnd, cur.end);
                    } else {
                        break;
                    }
                }

                merged = true;
            }
        }

        if (!merged) {
            // Try to merge with the current pos interval or insert
            Range1D& cur = (*this)[pos];
            SizeT gapCur = calc_gap(newRange, cur); // gap between new and cur
            SizeT thresholdCur = calc_threshold(newRange, cur);

            if (gapCur <= thresholdCur) {
                cur.start = std::min(cur.start, newRange.start);
                cur.end = std::max(cur.end, newRange.end);
                m_overallMaxEnd = std::max(m_overallMaxEnd, cur.end);

                size_t writeIdx = pos;
                while (writeIdx + 1 < this->size()) {
                    Range1D& cur2 = (*this)[writeIdx];
                    Range1D& nxt = (*this)[writeIdx + 1];
                    SizeT gap = calc_gap(cur2, nxt);
                    SizeT threshold = calc_threshold(cur2, nxt);
                    if (gap <= threshold) {
                        cur2.end = std::max(cur2.end, nxt.end);
                        this->erase(this->begin() + (writeIdx + 1));
                        m_overallMaxEnd = std::max(m_overallMaxEnd, cur2.end);
                    } else {
                        break;
                    }
                }

                merged = true;
            } else {
                this->insert(this->begin() + pos, newRange);
                m_overallMaxEnd = std::max(m_overallMaxEnd, newRange.end);
                merged = true;
            }
        }

        if (outMinStart) {
            *outMinStart = this->front().start;
        }
        if (outMaxEnd) {
            *outMaxEnd = m_overallMaxEnd;
        }
    }

    SizeT VecRange1D::GetOverallMaxEnd() const {
        return m_overallMaxEnd;
    }

    SizeT VecRange1D::GetOverallMinStart() const {
        if (this->empty()) return 0;
        return this->front().start;
    }
} // namespace MobileGL