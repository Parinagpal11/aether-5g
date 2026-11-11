#pragma once
#include <compare>
#include <cstdint>
#include <map>
#include <vector>

namespace aether::mac {
struct UeId {
  std::uint32_t value;
  auto operator<=>(const UeId&) const = default;
};
enum class Policy { round_robin, proportional_fair, qos };
struct Ue {
  UeId id;
  std::uint64_t queued = 0, served = 0;
  unsigned quality = 7, priority = 1;
  bool active = true;
  double average = 1.0;
};
struct Grant {
  UeId id;
  unsigned first_prb, prbs;
  std::uint64_t bytes;
  bool operator==(const Grant&) const = default;
};
// Deliberately approximate information capacity, NOT a 38.214 TBS implementation.
std::uint64_t capacity(unsigned quality, unsigned prbs);
double fairness(const std::vector<std::uint64_t>& values);
class SlotClock {
 public:
  explicit SlotClock(unsigned numerology);
  std::uint64_t tick();
  std::uint64_t slot() const { return slot_; }
  unsigned duration_ns() const { return duration_; }
 private:
  std::uint64_t slot_ = 0;
  unsigned duration_;
};
// Owns all state. Calls must be serialized by the caller.
class Scheduler {
 public:
  explicit Scheduler(Policy policy);
  void add(Ue ue);
  void remove(UeId id);
  void enqueue(UeId id, std::uint64_t bytes);
  void configure(UeId id, unsigned quality, unsigned priority, bool active);
  std::vector<Grant> schedule(unsigned available_prbs);
  const std::map<UeId, Ue>& state() const { return ues_; }
 private:
  Policy policy_;
  std::map<UeId, Ue> ues_;
  UeId cursor_{0};
};
}
