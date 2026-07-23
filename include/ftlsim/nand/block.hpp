#pragma once

#include <cstdint>
#include <vector>

#include "ftlsim/nand/page.hpp"

namespace ftlsim {

/// A NAND block: a fixed number of pages that must be programmed sequentially
/// and can only be reclaimed as a unit (erase). Tracks the erase count that
/// wear leveling works against.
class Block {
public:
    explicit Block(int pages_per_block);

    int pages_per_block() const { return static_cast<int>(pages_.size()); }
    const Page& page(int index) const { return pages_[static_cast<std::size_t>(index)]; }

    int valid_pages() const { return valid_; }
    int invalid_pages() const { return invalid_; }
    /// Pages past the write pointer -- erased and not yet programmed.
    int free_pages() const { return pages_per_block() - write_pointer_; }
    int write_pointer() const { return write_pointer_; }

    bool is_full() const { return write_pointer_ == pages_per_block(); }
    /// Fully erased and untouched since: eligible to become an open block.
    bool is_clean() const { return write_pointer_ == 0; }

    std::uint64_t erase_count() const { return erase_count_; }
    /// Timestep of the last program/invalidate, used by the cost-benefit policy.
    std::uint64_t last_modified() const { return last_modified_; }

    /// Program the next free page. Returns the page index, or -1 if full.
    int append(std::int64_t lpn, std::uint64_t timestep);
    /// Invalidate a previously programmed page.
    void invalidate(int page_index, std::uint64_t timestep);
    /// Erase the block: every page returns to Free and the erase count rises.
    void erase(std::uint64_t timestep);

private:
    std::vector<Page> pages_;
    int write_pointer_ = 0;
    int valid_ = 0;
    int invalid_ = 0;
    std::uint64_t erase_count_ = 0;
    std::uint64_t last_modified_ = 0;
};

}  // namespace ftlsim
