#pragma once

namespace ytta::m1 {

inline bool pin_current_thread_to_core(int /*core*/) { return false; }

}  // namespace ytta::m1
