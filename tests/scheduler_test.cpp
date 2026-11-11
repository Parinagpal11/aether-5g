#include "aether/scheduler.hpp"
#include <stdexcept>
void check(bool ok) { if (!ok) throw std::runtime_error("legacy invariant failed"); }
#include <cstdint>
#include <vector>
int main() {
  std::vector<aether::UE> ues{{1, 1000}, {2, 1000}};
  aether::RoundRobinScheduler scheduler(500);
  auto a = scheduler.schedule(ues);
  auto b = scheduler.schedule(ues);
  check(a.size() == 1 && a[0].ue_id == 1 && a[0].bytes == 500);
  check(b.size() == 1 && b[0].ue_id == 2 && b[0].bytes == 500);
  check(ues[0].queued_bytes == 500 && ues[1].queued_bytes == 500);
  std::vector<aether::UE> empty;
  check(scheduler.schedule(empty).empty());
  aether::RoundRobinScheduler zero(0);
  check(zero.schedule(ues).empty());
}
