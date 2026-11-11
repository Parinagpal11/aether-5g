#include "aether/scheduler.hpp"
#include <iostream>
#include <vector>
int main() {
  std::vector<aether::UE> ues{{1, 1800}, {2, 1200}, {3, 900}};
  aether::RoundRobinScheduler scheduler(1000);
  for (int slot = 0; slot < 5; ++slot) {
    const auto grants = scheduler.schedule(ues);
    std::cout << "slot=" << slot;
    for (const auto& grant : grants)
      std::cout << " ue=" << grant.ue_id << ":" << grant.bytes;
    std::cout << '\n';
  }
}
