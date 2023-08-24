#ifndef GRAPHIR_GRAPH_NODEMARKER_H
#define GRAPHIR_GRAPH_NODEMARKER_H

#include "graphir/Graph/Node.h"

namespace graphir {

class Graph;

class NodeMarkerBase {
 protected:
  using MarkerTy = typename Node::MarkerTy;

 private:
  MarkerTy marker_min_, marker_max_;

 public:
  NodeMarkerBase(Graph& graph, unsigned num_state);
  MarkerTy get(Node* node);
  void set(Node* node, MarkerTy val);
};

template <class T>
struct NodeMarker : public NodeMarkerBase {
  NodeMarker(Graph& graph, unsigned num_state)
      : NodeMarkerBase(graph, num_state) {}

  T get(Node* node) { return static_cast<T>(NodeMarkerBase::get(node)); }
  void set(Node* node, T val) {
    NodeMarkerBase::set(node,
                        static_cast<typename NodeMarkerBase::MarkerTy>(val));
  }
};

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_NODEMARKER_H