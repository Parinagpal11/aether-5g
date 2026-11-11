#include "aether/mac/scheduler.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
using namespace aether::mac;
int main(){
 std::cout<<"policy,ues,samples,p50_us,p95_us,p99_us,bytes\n";
 for(auto policy:{Policy::round_robin,Policy::proportional_fair,Policy::qos})for(unsigned n:{1U,10U,100U}){
  Scheduler s(policy);for(unsigned id=1;id<=n;++id)s.add({{id}});
  std::vector<double> times;std::uint64_t bytes=0;
  for(int i=0;i<11000;++i){for(unsigned id=1;id<=n;++id)s.enqueue({id},1000);
   auto start=std::chrono::steady_clock::now();auto grants=s.schedule(100);auto end=std::chrono::steady_clock::now();
   if(i>=1000){times.push_back(std::chrono::duration<double,std::micro>(end-start).count());for(auto g:grants)bytes+=g.bytes;}}
  std::sort(times.begin(),times.end());
  std::cout<<static_cast<int>(policy)<<','<<n<<','<<times.size()<<','<<times[4999]<<','<<times[9499]<<','<<times[9899]<<','<<bytes<<'\n';
 }
}
