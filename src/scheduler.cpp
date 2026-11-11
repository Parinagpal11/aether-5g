#include "aether/scheduler.hpp"
#include <algorithm>
namespace aether {
std::vector<Grant> RoundRobinScheduler::schedule(std::vector<UE>& ues) {
  std::vector<Grant> grants;
  if (ues.empty() || capacity_ == 0) return grants;
  std::uint64_t remaining = capacity_;
  const auto n = ues.size();
  cursor_ %= n;
  std::size_t examined = 0;
  while (remaining > 0 && examined < n) {
    auto& ue = ues[cursor_];
    const auto allocation = std::min(ue.queued_bytes, remaining);
    if (allocation > 0) {
      ue.queued_bytes -= allocation;
      ue.served_bytes += allocation;
      remaining -= allocation;
      grants.push_back({ue.id, allocation});
    }
    cursor_ = (cursor_ + 1) % n;
    ++examined;
  }
  return grants;
}
}
