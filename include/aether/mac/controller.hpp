#pragma once

#include <array>
#include <optional>

#include "aether/mac/harq.hpp"
#include "aether/mac/scheduler.hpp"

namespace aether::mac {
enum class Direction { downlink, uplink };
struct BufferStatusReport {
  std::uint64_t sequence, generated_slot;
  // Simulation-only exact cumulative arrival and retirement watermarks.
  // outstanding includes reserved/in-flight bytes at report generation.
  std::uint64_t admitted, retired, outstanding;
};
struct Accounting {
  std::uint64_t admitted, queued, in_flight, delivered, failed;
  bool operator==(const Accounting&) const = default;
};
struct Transmission {
  Direction direction;
  Grant grant;
  AttemptToken token;
  std::uint64_t slot;
  bool operator==(const Transmission&) const = default;
};
struct Feedback {
  Direction direction;
  UeId ue;
  AttemptToken token;
  bool ack;
};
struct ControllerConfig {
  HarqConfig harq{};
  std::uint64_t bsr_max_age = 20;
  std::uint64_t max_outstanding_bytes = 1'000'000'000;
};

// Deterministic, single-owner MAC controller. Original Scheduler is unmodified.
class MacController {
 public:
  explicit MacController(Policy policy = Policy::round_robin, ControllerConfig config = {});
  void add(UeId id);
  void configure(Direction direction, UeId id, unsigned quality, unsigned priority, bool active);
  void begin_slot(std::uint64_t slot);  // Strictly increasing. Expires HARQ first.
  void enqueue_downlink(UeId id, std::uint64_t bytes);
  void report_uplink(UeId id, BufferStatusReport report);
  std::vector<Transmission> schedule(Direction direction, unsigned prbs);
  FeedbackResult feedback(const Feedback& feedback);
  Accounting accounting(Direction direction, UeId id) const;
  const HarqManager& harq(Direction direction, UeId id) const;
  bool invariants_hold() const;
  std::uint64_t slot() const;

 private:
  struct Link {
    explicit Link(HarqConfig config) : harq(config) {}
    HarqManager harq;
    std::optional<BufferStatusReport> report;
    std::uint64_t admitted = 0, delivered = 0, failed = 0;
    unsigned quality = 7, priority = 1;
    bool active = true;
  };
  struct Plane {
    explicit Plane(Policy policy) : scheduler(policy) {}
    Scheduler scheduler;
    std::map<UeId, Link> links;
    bool scheduled = false;
    UeId retry_cursor{0};
  };
  static unsigned index(Direction direction);
  bool fresh(const Link& link) const;
  ControllerConfig config_;
  std::array<Plane, 2> planes_;
  std::optional<std::uint64_t> slot_;
};
}  // namespace aether::mac
