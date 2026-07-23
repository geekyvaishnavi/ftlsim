#include "ftlsim/ftl/mapping.hpp"

#include <stdexcept>

namespace ftlsim {

MappingTable::MappingTable(std::int64_t logical_pages)
    : table_(static_cast<std::size_t>(logical_pages), kInvalidPpn) {
    if (logical_pages <= 0) {
        throw std::invalid_argument("mapping table needs at least one logical page");
    }
}

std::int64_t MappingTable::lookup(std::int64_t lpn) const {
    if (lpn < 0 || lpn >= logical_pages()) {
        return kInvalidPpn;
    }
    return table_[static_cast<std::size_t>(lpn)];
}

void MappingTable::map(std::int64_t lpn, std::int64_t ppn) {
    if (lpn < 0 || lpn >= logical_pages()) {
        throw std::out_of_range("logical page out of range");
    }
    std::int64_t& entry = table_[static_cast<std::size_t>(lpn)];
    if (entry == kInvalidPpn && ppn != kInvalidPpn) {
        ++mapped_;
    }
    entry = ppn;
}

void MappingTable::unmap(std::int64_t lpn) {
    if (lpn < 0 || lpn >= logical_pages()) {
        return;
    }
    std::int64_t& entry = table_[static_cast<std::size_t>(lpn)];
    if (entry != kInvalidPpn) {
        --mapped_;
    }
    entry = kInvalidPpn;
}

}  // namespace ftlsim
