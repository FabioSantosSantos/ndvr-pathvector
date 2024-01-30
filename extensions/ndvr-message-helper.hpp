#ifndef _NDVR_HELPER_HPP_
#define _NDVR_HELPER_HPP_

#include "ndvr-message.pb.h"
#include "ibf.hpp"


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
      nextHopProto->set_count(nextHop.Cost());

      for (size_t bit: nextHop.GetBitsIbf()){
        nextHopProto->add_bits_ibf(bit);
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

  std::cout << "###### DecodeDvInfo call"  << std::endl;

  for (int i = 0; i < dvinfo_proto.entry_size(); ++i) {
    
    const auto& entry = dvinfo_proto.entry(i);
    auto prefix = entry.prefix();
    auto seq = entry.seq();
    auto originator = entry.originator();

    std::cout << "###### DecodeDvInfo Prefix" << prefix << std::endl;
    std::cout << "###### DecodeDvInfo seq" << seq << std::endl;
    std::cout << "###### DecodeDvInfo originator" << originator << std::endl;
    
    std::vector<NextHop> nextHops;
    
    for (int j = 0; j < entry.nexthop_size(); ++j) {
      proto::DvInfo_NextHop nextHopProto = entry.nexthop(j);

      std::vector<size_t> bits;
      //NextHop(std::string nexthop_id, int count, std::vector<size_t> bits_ibf)

      
      for (int k = 0; k < nextHopProto.bits_ibf_size(); ++k) {
        bits.push_back(nextHopProto.bits_ibf(k));
      }

      NextHop nextHop = NextHop(nextHopProto.nexthop_id(), nextHopProto.count(), bits);

      std::cout << "###### DecodeDvInfo nexthop_id = " << nextHopProto.nexthop_id() << std::endl;
      std::cout << "###### DecodeDvInfo bits = " << join(bits, ",") << std::endl;
      std::cout << "###### DecodeDvInfo next hops cost = " << nextHop.Cost() << std::endl;

      nextHops.push_back(nextHop);
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
