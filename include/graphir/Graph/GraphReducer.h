#ifndef GRAPHIR_GRAPH_GRAPHREDUCER_H
#define GRAPHIR_GRAPH_GRAPHREDUCER_H

#include <utility>

#include "graphir/Graph/Graph.h"
#include "graphir/Graph/Node.h"
#include "graphir/Graph/NodeMarker.h"

namespace graphir {

struct GraphReduction {
  explicit GraphReduction(Node* node = nullptr) : replacement_node_(node) {}

  Node* Replacement() const { return replacement_node_; }
  bool Changed() const { return Replacement() != nullptr; }

 private:
  Node* replacement_node_;
};

namespace detail {

struct ReducerConcept {
  virtual const char* name() const = 0;

  virtual GraphReduction Reduce(Node* node) = 0;
};

template <class ReducerT, class... CtorArgs>
struct ReducerModel : public ReducerConcept {
  ReducerModel(CtorArgs&&... args)
      : reducer_(std::forward<CtorArgs>(args)...) {}

  GraphReduction Reduce(Node* node) override { return reducer_.Reduce(node); }

  const char* name() const override { return ReducerT::name(); }

 private:
  ReducerT reducer_;
};

}  // namespace detail

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_GRAPHREDUCER_H