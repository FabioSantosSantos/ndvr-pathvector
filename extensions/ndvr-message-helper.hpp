#ifndef _NDVR_HELPER_HPP_
#define _NDVR_HELPER_HPP_

#include "ndvr-message.pb.h"

namespace ndn {
namespace ndvr {

inline void EncodeDvInfo(RoutingTable& v, proto::DvInfo* dvinfo_proto) {
  for (auto it = v.begin(); it != v.end(); ++it) {
    auto* entry = dvinfo_proto->add_entry();
    entry->set_prefix(it->first);
    entry->set_seq(it->second.GetSeqNum());
    entry->set_cost(it->second.GetBestCost());
    entry->set_originator(it->second.GetOriginator());
    //entry->set_bestnexthop(it->second.GetLearnedFrom());
    //entry->set_sec_cost(it->second.GetSecondBestCost());
    
    NextHop nextHop = it->second.GetNextHops2();
    proto::DvInfo_NextHop *next_hop = new proto::DvInfo_NextHop();
    //next_hop->set_cost(nextHop.GetCost());

    for (std::string router_id: nextHop.GetRouterIds()) {
      next_hop->add_router_id(router_id);
    }

    entry->set_allocated_next_hops(next_hop);

  }
}

inline void EncodeDvInfo(RoutingTable& v, std::string& out) {
  proto::DvInfo dvinfo_proto;
  EncodeDvInfo(v, &dvinfo_proto);
  dvinfo_proto.AppendToString(&out);
}

template <typename T>
std::string join(const T& v, const std::string& delim) {
    std::ostringstream s;
    for (const auto& i : v) {
        if (&i != &v[0]) {
            s << delim;
        }
        s << i;
    }
    return s.str();
}

inline RoutingTable DecodeDvInfo(const proto::DvInfo& dvinfo_proto) {
  RoutingTable dvinfo;
  for (int i = 0; i < dvinfo_proto.entry_size(); ++i) {
    
    const auto& entry = dvinfo_proto.entry(i);
    auto prefix = entry.prefix();
    auto seq = entry.seq();
    auto originator = entry.originator();
    auto cost = entry.cost();
    //auto bestnexthop = entry.bestnexthop();
    //auto sec_cost = entry.sec_cost();
    std::vector<std::string> ids;
    
    for (int j = 0; j < entry.next_hops().router_id_size(); ++j) {
      ids.push_back(entry.next_hops().router_id(j));
    }

    std::cout << "###### DecodeDvInfo next hops = " << join(ids, ",") << std::endl;
     //std::cout << "### >> prefix     :" << routerPrefix_Uri << std::endl;

    NextHop nextHop = NextHop(ids);
    RoutingEntry re = RoutingEntry(prefix, seq, originator, cost, nextHop);

    dvinfo.emplace(prefix, re);
  }
  return dvinfo;
}

inline RoutingTable DecodeDvInfo(const void* buf, size_t buf_size) {
  proto::DvInfo dvinfo_proto;
  if (!dvinfo_proto.ParseFromArray(buf, buf_size)) {
    RoutingTable res;
    return res;
  }
  return DecodeDvInfo(dvinfo_proto);
}
}  // namespace ndvr
}  // namespace ndn

#endif  // _NDVR_HELPER_HPP_
