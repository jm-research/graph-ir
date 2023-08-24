#include "graphir/Graph/NodeMarker.h"

#include "graphir/Graph/Graph.h"

namespace graphir {

NodeMarkerBase::NodeMarkerBase(Graph& g, unsigned num_state)
    : marker_min_(g.marker_max_), marker_max_(g.marker_max_ += num_state) {
  assert(num_state != 0U);
  assert(marker_min_ < marker_max_ && "Wraparound!");
}

NodeMarkerBase::MarkerTy NodeMarkerBase::get(Node* N) {
  auto Data = N->marker_data_;
  if (Data < marker_min_) {
    return 0;
  }
  assert(Data < marker_max_ && "Using an old NodeMarker?");
  return Data - marker_min_;
}

void NodeMarkerBase::set(Node* n, NodeMarkerBase::MarkerTy new_marker) {
  assert(new_marker < (marker_max_ - marker_min_));
  assert(n->marker_data_ < marker_max_);
  n->marker_data_ = (marker_min_ + new_marker);
}

}  // namespace graphir