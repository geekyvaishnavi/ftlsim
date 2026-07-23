#include "ftlsim/nand/page.hpp"

namespace ftlsim {

void Page::program(std::int64_t lpn) {
    state_ = PageState::Valid;
    lpn_ = lpn;
}

void Page::invalidate() {
    state_ = PageState::Invalid;
}

void Page::reset() {
    state_ = PageState::Free;
    lpn_ = kInvalidLpn;
}

char page_state_char(PageState state) {
    switch (state) {
        case PageState::Free: return 'f';
        case PageState::Valid: return 'v';
        case PageState::Invalid: return 'i';
    }
    return '?';
}

}  // namespace ftlsim
