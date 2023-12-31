/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2020,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NFD_DAEMON_FW_ASF_MEASUREMENTS_HPP
#define NFD_DAEMON_FW_ASF_MEASUREMENTS_HPP

#include "fw/strategy-info.hpp"
#include "table/measurements-accessor.hpp"

#include <ndn-cxx/util/rtt-estimator.hpp>

namespace nfd {
namespace fw {
namespace asf {

/** \brief Strategy information for each face in a namespace
*/
class FaceInfo
{
public:
  explicit
  FaceInfo(shared_ptr<const ndn::util::RttEstimator::Options> opts)
    : m_rttEstimator(std::move(opts))
  {
  }

  bool
  isTimeoutScheduled() const
  {
    return !!m_timeoutEvent;
  }

  time::nanoseconds
  scheduleTimeout(const Name& interestName, scheduler::EventCallback cb);

  void
  cancelTimeout(const Name& prefix);

  void
  recordNack(const Name& interestName)
  {
    m_lastRtt = RTT_NACK;
    if (m_lastInterestName.isPrefixOf(interestName)) {
      m_timeoutEvent.cancel();
    }
    //TODO: isNacked?
    //m_isNacked = true;
  }

  bool isNacked() const
  {
    return getLastRtt() == RTT_NACK;
  }

  void
  recordRtt(time::nanoseconds rtt)
  {
    m_lastRtt = rtt;
    m_rttEstimator.addMeasurement(rtt);
  }

  void
  recordTimeout(const Name& interestName)
  {
    m_lastRtt = RTT_TIMEOUT;
    if (m_lastInterestName.isPrefixOf(interestName)) {
      m_timeoutEvent.cancel();
    }
  }
  //TODO: Clean up code to use this instead of manually checking
  bool
  hasTimeout() const
  {
    return getLastRtt() == RTT_TIMEOUT;
  }

  time::nanoseconds
  getLastRtt() const
  {
    return m_lastRtt;
  }

  time::nanoseconds
  getSrtt() const
  {
    return m_rttEstimator.getSmoothedRtt();
  }

  size_t
  getNSilentTimeouts() const
  {
    return m_nSilentTimeouts;
  }

  void
  setNSilentTimeouts(size_t nSilentTimeouts)
  {
    m_nSilentTimeouts = nSilentTimeouts;
  }

  void
  markLastTimeForwarded()
  {
    m_lastTimeForwarded = time::steady_clock::now();
  }

  time::steady_clock::TimePoint
  getLastTimeForwarded()
  {
    return m_lastTimeForwarded;
  }

public:
  static const time::nanoseconds RTT_NO_MEASUREMENT;
  static const time::nanoseconds RTT_TIMEOUT;
  static const time::nanoseconds RTT_NACK;

private:
  ndn::util::RttEstimator m_rttEstimator;
  time::nanoseconds m_lastRtt = RTT_NO_MEASUREMENT;
  Name m_lastInterestName;
  size_t m_nSilentTimeouts = 0;
  //bool m_isNacked = false;
  time::steady_clock::TimePoint m_lastTimeForwarded;

  // Timeout associated with measurement
  scheduler::ScopedEventId m_measurementExpiration;
  friend class NamespaceInfo;

  // RTO associated with Interest
  scheduler::ScopedEventId m_timeoutEvent;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** \brief Stores strategy information about each face in this namespace
 */
class NamespaceInfo : public StrategyInfo
{
public:
  static constexpr int
  getTypeId()
  {
    return 1030;
  }

  explicit
  NamespaceInfo(shared_ptr<const ndn::util::RttEstimator::Options> opts)
    : m_rttEstimatorOpts(std::move(opts))
  {
  }

  FaceInfo*
  getFaceInfo(FaceId faceId);

  FaceInfo&
  getOrCreateFaceInfo(FaceId faceId);

  void
  extendFaceInfoLifetime(FaceInfo& info, FaceId faceId);

  bool
  isProbingDue() const
  {
    return m_isProbingDue;
  }

  void
  setIsProbingDue(bool isProbingDue)
  {
    m_isProbingDue = isProbingDue;
  }

  bool
  isFirstProbeScheduled() const
  {
    return m_isFirstProbeScheduled;
  }

  void
  setIsFirstProbeScheduled(bool isScheduled)
  {
    m_isFirstProbeScheduled = isScheduled;
  }

  struct BestPrefixFace
  {
    Face * primaryFace = nullptr;
    Face * bestFace = nullptr;
		ndn::time::system_clock::TimePoint changeTime;
    bool contest = false;
  };

  BestPrefixFace*
  getFaceContest() {
    if (m_bestFace == nullptr) {
      m_bestFace = new BestPrefixFace();
    }
    return m_bestFace;
  }

  void
  setFaceContest(BestPrefixFace* fc){
    m_bestFace = fc;
  }

private:
  std::unordered_map<FaceId, FaceInfo> m_fiMap;
  shared_ptr<const ndn::util::RttEstimator::Options> m_rttEstimatorOpts;
  bool m_isProbingDue = false;
  bool m_isFirstProbeScheduled = false;
  BestPrefixFace* m_bestFace = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** \brief Helper class to retrieve and create strategy measurements
 */
class AsfMeasurements : noncopyable
{
public:
  explicit
  AsfMeasurements(MeasurementsAccessor& measurements);

  FaceInfo*
  getFaceInfo(const fib::Entry& fibEntry, const Interest& interest, FaceId faceId);

  FaceInfo&
  getOrCreateFaceInfo(const fib::Entry& fibEntry, const Interest& interest, FaceId faceId);

  NamespaceInfo*
  getNamespaceInfo(const Name& prefix);

  NamespaceInfo&
  getOrCreateNamespaceInfo(const fib::Entry& fibEntry, const Interest& interest);

private:
  void
  extendLifetime(measurements::Entry& me);

public:
  static constexpr time::microseconds MEASUREMENTS_LIFETIME = 5_min;

private:
  MeasurementsAccessor& m_measurements;
  shared_ptr<const ndn::util::RttEstimator::Options> m_rttEstimatorOpts;
};

struct FaceStats
{
  Face* face;
  time::nanoseconds rtt;
  time::nanoseconds srtt;
  uint64_t cost;
};

struct FaceStatsCompare
{
  bool
  operator()(const FaceStats& lhs, const FaceStats& rhs) const
  {
    time::nanoseconds zeroValue = time::nanoseconds{0};
    time::nanoseconds lhsRtt = lhs.rtt >= zeroValue ? zeroValue : -lhs.rtt;
    time::nanoseconds rhsRtt = rhs.rtt >= zeroValue ? zeroValue : -rhs.rtt; 

		time::nanoseconds lhsSrtt = lhs.srtt == FaceInfo::RTT_NO_MEASUREMENT ? time::nanoseconds::max() : lhs.srtt;
    time::nanoseconds rhsSrtt = rhs.srtt == FaceInfo::RTT_NO_MEASUREMENT ? time::nanoseconds::max() : rhs.srtt;

		FaceId lhsFaceId = lhs.face->getId();
		FaceId rhsFaceId = rhs.face->getId();

		return std::tie(lhsRtt, lhsSrtt, lhs.cost, lhsFaceId) < std::tie(rhsRtt, rhsSrtt, rhs.cost, rhsFaceId);
  }
};


} // namespace asf
} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_ASF_MEASUREMENTS_HPP
