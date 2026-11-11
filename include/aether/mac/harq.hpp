#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace aether::mac {
enum class HarqState { idle, waiting_ack, retransmission_ready, failed };
enum class FeedbackResult { ignored, acknowledged, retry_ready, failed };
struct HarqConfig {
  unsigned processes = 4;
  unsigned max_attempts = 3;  // Includes the initial transmission.
  std::uint64_t timeout_slots = 4;
};
struct AttemptToken {
  unsigned process = 0;
  std::uint64_t generation = 0;
  unsigned attempt = 0;
  bool operator==(const AttemptToken&) const = default;
};
struct HarqProcess {
  HarqState state = HarqState::idle;
  std::uint64_t generation = 0, bytes = 0, sent_slot = 0, deadline = 0;
  unsigned attempts = 0;
};
struct Completion {
  FeedbackResult result = FeedbackResult::ignored;
  std::uint64_t bytes = 0;
};

// One manager per UE/direction. Only its owner may mutate it.
class HarqManager {
 public:
  explicit HarqManager(HarqConfig config = {});
  bool available() const;
  std::optional<unsigned> ready() const;
  AttemptToken start(std::uint64_t bytes, std::uint64_t slot);
  AttemptToken retransmit(unsigned process, std::uint64_t slot);
  Completion feedback(AttemptToken token, bool ack, std::uint64_t slot);
  std::uint64_t expire(std::uint64_t slot);  // Newly failed bytes only.
  std::uint64_t pending() const;
  const std::vector<HarqProcess>& processes() const { return processes_; }

 private:
  Completion negative(HarqProcess& process);
  void validate_slot(std::uint64_t slot) const;
  HarqConfig config_;
  std::vector<HarqProcess> processes_;
};
}  // namespace aether::mac
