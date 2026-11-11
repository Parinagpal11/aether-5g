#include "aether/mac/scheduler.hpp"
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>
using namespace aether::mac;
unsigned number(const char* text) {
  std::string_view s(text); unsigned n=0;auto [p,e]=std::from_chars(s.data(),s.data()+s.size(),n);
  if(e!=std::errc{}||p!=s.data()+s.size())throw std::invalid_argument("invalid integer");return n;
}
int main(int argc,char** argv) {
 try {
  if(argc!=6)throw std::invalid_argument("usage: aether_slots rr|pf|qos UEs slots PRBs bytes_per_UE_per_slot");
  std::string_view name(argv[1]);Policy p;
  if(name=="rr")p=Policy::round_robin;else if(name=="pf")p=Policy::proportional_fair;else if(name=="qos")p=Policy::qos;else throw std::invalid_argument("unknown policy");
  auto n=number(argv[2]),slots=number(argv[3]),prbs=number(argv[4]),arrival=number(argv[5]);
  if(n>10000||slots>10000000||prbs>275)throw std::invalid_argument("scenario limit exceeded");
  Scheduler s(p);SlotClock clock(0);for(unsigned i=1;i<=n;++i)s.add({{i}});
  std::cout<<"slot,ue,first_prb,prbs,bytes\n";
  for(unsigned t=0;t<slots;++t){for(unsigned i=1;i<=n;++i)s.enqueue({i},arrival);auto slot=clock.tick();
    for(auto g:s.schedule(prbs))std::cout<<slot<<','<<g.id.value<<','<<g.first_prb<<','<<g.prbs<<','<<g.bytes<<'\n';}
  std::vector<std::uint64_t> values;for(const auto& [id,u]:s.state())values.push_back(u.served);
  std::cerr<<"{\"slots\":"<<clock.slot()<<",\"fairness\":"<<fairness(values)<<"}\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
