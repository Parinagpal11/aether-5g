#include "aether/mac/bidirectional.hpp"

#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {
unsigned number(const char* text) {
  std::string_view s(text);
  unsigned result = 0;
  const auto [end, error] = std::from_chars(s.data(), s.data() + s.size(), result);
  if (error != std::errc{} || end != s.data() + s.size())
    throw std::invalid_argument("invalid nonnegative integer");
  return result;
}
}  // namespace
int main(int argc, char** argv) {
  try {
    if (argc != 7 && argc != 8)
      throw std::invalid_argument(
          "usage: aether_bidirectional rr|pf|qos UEs traffic_slots seed nack_per_mille "
          "silent_per_mille [--trace]");
    aether::mac::BidirectionalConfig config;
    const std::string_view policy(argv[1]);
    if (policy == "rr")
      config.policy = aether::mac::Policy::round_robin;
    else if (policy == "pf")
      config.policy = aether::mac::Policy::proportional_fair;
    else if (policy == "qos")
      config.policy = aether::mac::Policy::qos;
    else
      throw std::invalid_argument("invalid policy");
    config.ues = number(argv[2]);
    config.traffic_slots = number(argv[3]);
    config.phy.seed = number(argv[4]);
    config.phy.nack_per_mille = number(argv[5]);
    config.phy.silent_per_mille = number(argv[6]);
    if (argc == 8 && std::string_view(argv[7]) != "--trace")
      throw std::invalid_argument("invalid trace option");
    const auto r = aether::mac::run_bidirectional(config, argc == 8 ? &std::cout : nullptr);
    auto& out = argc == 8 ? std::cerr : std::cout;
    out << "{\"slots\":" << r.slots << ",\"transmissions\":" << r.transmissions
        << ",\"retries\":" << r.retries << ",\"trace_hash\":" << r.trace_hash;
    for (unsigned d = 0; d < 2; ++d) {
      const auto& a = r.totals[d];
      out << ",\"" << (d ? "ul" : "dl") << "\":{\"admitted\":" << a.admitted
          << ",\"queued\":" << a.queued << ",\"in_flight\":" << a.in_flight
          << ",\"delivered\":" << a.delivered << ",\"failed\":" << a.failed << '}';
    }
    out << "}\n";
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
