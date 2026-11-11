#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace aether {
struct UE { std::uint32_t id; std::uint64_t queued_bytes; std::uint64_t served_bytes = 0; };
struct Grant { std::uint32_t ue_id; std::uint64_t bytes; };
class RoundRobinScheduler {
 public:
  explicit RoundRobinScheduler(std::uint64_t bytes_per_slot) : capacity_(bytes_per_slot) {}
  std::vector<Grant> schedule(std::vector<UE>& ues);
 private:
  std::uint64_t capacity_;
  std::size_t cursor_ = 0;
};
}
