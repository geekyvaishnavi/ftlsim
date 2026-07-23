#pragma once

#include <cstdint>

namespace ftlsim {

/// A physical NAND page is in exactly one of three states. Flash cannot be
/// overwritten in place, so a page moves Free -> Valid -> Invalid and only
/// returns to Free when the whole containing block is erased.
enum class PageState : std::uint8_t { Free, Valid, Invalid };

inline constexpr std::int64_t kInvalidLpn = -1;
inline constexpr std::int64_t kInvalidPpn = -1;

class Page {
public:
    PageState state() const { return state_; }
    std::int64_t lpn() const { return lpn_; }

    bool is_free() const { return state_ == PageState::Free; }
    bool is_valid() const { return state_ == PageState::Valid; }
    bool is_invalid() const { return state_ == PageState::Invalid; }

    /// Program this page with the given logical page number.
    void program(std::int64_t lpn);
    /// Mark the data here as superseded by a newer copy elsewhere.
    void invalidate();
    /// Return the page to the free state (only legal as part of a block erase).
    void reset();

private:
    PageState state_ = PageState::Free;
    std::int64_t lpn_ = kInvalidLpn;
};

/// Single-character rendering used by the JSONL state dump: 'f', 'v', 'i'.
char page_state_char(PageState state);

}  // namespace ftlsim
