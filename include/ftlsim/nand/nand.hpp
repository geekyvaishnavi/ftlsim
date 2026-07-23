#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "ftlsim/nand/block.hpp"

namespace ftlsim {

/// The flash array: a flat vector of blocks addressed by physical page number
/// (ppn = block_index * pages_per_block + page_index).
class Nand {
public:
    Nand(int block_count, int pages_per_block);

    int block_count() const { return static_cast<int>(blocks_.size()); }
    int pages_per_block() const { return pages_per_block_; }
    std::int64_t total_pages() const {
        return static_cast<std::int64_t>(block_count()) * pages_per_block_;
    }

    Block& block(int index) { return blocks_[static_cast<std::size_t>(index)]; }
    const Block& block(int index) const { return blocks_[static_cast<std::size_t>(index)]; }

    int block_of(std::int64_t ppn) const { return static_cast<int>(ppn / pages_per_block_); }
    int page_of(std::int64_t ppn) const { return static_cast<int>(ppn % pages_per_block_); }
    std::int64_t ppn_of(int block_index, int page_index) const {
        return static_cast<std::int64_t>(block_index) * pages_per_block_ + page_index;
    }

    const Page& page_at(std::int64_t ppn) const {
        return block(block_of(ppn)).page(page_of(ppn));
    }

    /// Program `lpn` into block `block_index`; returns the resulting ppn or -1.
    std::int64_t program(int block_index, std::int64_t lpn, std::uint64_t timestep);
    void invalidate(std::int64_t ppn, std::uint64_t timestep);
    void erase_block(int block_index, std::uint64_t timestep);

    std::int64_t valid_pages() const;
    std::uint64_t total_erases() const;

    /// Compact per-block page-state string ("fvi..."), for the JSONL dump.
    std::string state_string(int block_index) const;

private:
    int pages_per_block_;
    std::vector<Block> blocks_;
};

}  // namespace ftlsim
