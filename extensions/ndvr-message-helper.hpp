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
    entry->set_originator(it->second.GetOriginator());
   

    for (NextHop nextHop: it->second.GetNextHops2()) {
      proto::DvInfo_NextHop *nextHopProto = entry->add_nexthop();
      nextHopProto->set_nexthop_id(nextHop.GetNexthopId());

      for (std::string path_nexthop: nextHop.GetPathNexthop()){
        nextHopProto->add_path_nexthop(path_nexthop);
      }

    }

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
    
    std::vector<NextHop> nextHops;
    
    for (int j = 0; j < entry.nexthop_size(); ++j) {
      proto::DvInfo_NextHop nextHopProto = entry.nexthop(j);
      std::vector<std::string> ids;
      
      for (int i = 0; i < nextHopProto.path_nexthop_size(); ++i) {
        ids.push_back(nextHopProto.path_nexthop(i));
      }
      std::cout << "###### DecodeDvInfo next hops = " << join(ids, ",") << std::endl;

      nextHops.push_back(NextHop(nextHopProto.nexthop_id(), ids));
    }
    
    RoutingEntry re = RoutingEntry(prefix, seq, originator, nextHops);

    dvinfo.emplace(prefix, re);
  
  return dvinfo;
  }
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
