#include "ftlsim/nand/nand.hpp"

#include <stdexcept>

namespace ftlsim {

Nand::Nand(int block_count, int pages_per_block) : pages_per_block_(pages_per_block) {
    if (block_count <= 0 || pages_per_block <= 0) {
        throw std::invalid_argument("NAND geometry must be positive");
    }
    blocks_.reserve(static_cast<std::size_t>(block_count));
    for (int i = 0; i < block_count; ++i) {
        blocks_.emplace_back(pages_per_block);
    }
}

std::int64_t Nand::program(int block_index, std::int64_t lpn, std::uint64_t timestep) {
    const int page_index = block(block_index).append(lpn, timestep);
    if (page_index < 0) {
        return kInvalidPpn;
    }
    return ppn_of(block_index, page_index);
}

void Nand::invalidate(std::int64_t ppn, std::uint64_t timestep) {
    if (ppn == kInvalidPpn) {
        return;
    }
    block(block_of(ppn)).invalidate(page_of(ppn), timestep);
}

void Nand::erase_block(int block_index, std::uint64_t timestep) {
    block(block_index).erase(timestep);
}

std::int64_t Nand::valid_pages() const {
    std::int64_t total = 0;
    for (const Block& b : blocks_) {
        total += b.valid_pages();
    }
    return total;
}

std::uint64_t Nand::total_erases() const {
    std::uint64_t total = 0;
    for (const Block& b : blocks_) {
        total += b.erase_count();
    }
    return total;
}

std::string Nand::state_string(int block_index) const {
    const Block& b = block(block_index);
    std::string out;
    out.reserve(static_cast<std::size_t>(b.pages_per_block()));
    for (int i = 0; i < b.pages_per_block(); ++i) {
        out.push_back(page_state_char(b.page(i).state()));
    }
    return out;
}

}  // namespace ftlsim
