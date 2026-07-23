#pragma once

#include <cstdint>
#include <vector>

#include "ftlsim/nand/page.hpp"

namespace ftlsim {

/// Page-level logical-to-physical mapping table. One entry per logical page;
/// kInvalidPpn means the logical page has never been written.
class MappingTable {
public:
    explicit MappingTable(std::int64_t logical_pages);

    std::int64_t logical_pages() const { return static_cast<std::int64_t>(table_.size()); }

    /// Physical location of `lpn`, or kInvalidPpn if unmapped.
    std::int64_t lookup(std::int64_t lpn) const;
    void map(std::int64_t lpn, std::int64_t ppn);
    void unmap(std::int64_t lpn);

    /// Number of logical pages currently backed by a physical page.
    std::int64_t mapped_pages() const { return mapped_; }

private:
    std::vector<std::int64_t> table_;
    std::int64_t mapped_ = 0;
};

}  // namespace ftlsim
