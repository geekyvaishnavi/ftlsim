#include "ftlsim/nand/block.hpp"

namespace ftlsim {

Block::Block(int pages_per_block)
    : pages_(static_cast<std::size_t>(pages_per_block)) {}

int Block::append(std::int64_t lpn, std::uint64_t timestep) {
    if (is_full()) {
        return -1;
    }
    const int index = write_pointer_++;
    pages_[static_cast<std::size_t>(index)].program(lpn);
    ++valid_;
    last_modified_ = timestep;
    return index;
}

void Block::invalidate(int page_index, std::uint64_t timestep) {
    Page& page = pages_[static_cast<std::size_t>(page_index)];
    if (!page.is_valid()) {
        return;
    }
    page.invalidate();
    --valid_;
    ++invalid_;
    last_modified_ = timestep;
}

void Block::erase(std::uint64_t timestep) {
    for (Page& page : pages_) {
        page.reset();
    }
    write_pointer_ = 0;
    valid_ = 0;
    invalid_ = 0;
    ++erase_count_;
    last_modified_ = timestep;
}

}  // namespace ftlsim
