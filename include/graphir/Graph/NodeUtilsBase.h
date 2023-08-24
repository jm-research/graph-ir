#ifndef GRAPHIR_GRAPH_NODEUTILSBASE_H
#define GRAPHIR_GRAPH_NODEUTILSBASE_H

#include "graphir/Graph/Graph.h"
#include "graphir/Graph/Node.h"

namespace graphir {

template <IrOpcode::ID OP>
class NodePropertiesBase {
 protected:
  Node* node_ptr_;

  NodePropertiesBase(Node* node) : node_ptr_(node) {}

 public:
  operator bool() const { return node_ptr_ && node_ptr_->op_ == OP; }
};

template <IrOpcode::ID OP>
struct NodeProperties : public NodePropertiesBase<OP> {
  NodeProperties(Node* node) : NodePropertiesBase<OP>(node) {}
};

#define NODE_PROPERTIES(OP)           \
  template <>                         \
  struct NodeProperties<IrOpcode::OP> \
      : public NodePropertiesBase<IrOpcode::OP>

#define NODE_PROPERTIES_VIRT(OP, VIRTOP) \
  template <>                            \
  struct NodeProperties<IrOpcode::OP>    \
      : public NodeProperties<IrOpcode::VIRTOP>

#define NODE_PROP_BASE(OP, NODE)     NodePropertiesBase<IrOpcode::OP>(NODE)
#define NODE_PROP_VIRT(VIRTOP, NODE) NodeProperties<IrOpcode::VIRTOP>(NODE)

template <IrOpcode::ID OC>
struct NodeBuilder {
  NodeBuilder(Graph* graph) : graph_(graph) {}

  Node* Build() {
    auto* node = new Node(OC, {});
    graph_->InsertNode(node);
    return node;
  }

 private:
  Graph* graph_;
};

namespace internal {

template <IrOpcode::ID OC, class DerivedT = NodeBuilder<OC>>
class MemNodeBuilder {
  inline DerivedT& derived() { return *static_cast<DerivedT*>(this); }

 public:
  MemNodeBuilder(Graph* graph) : graph_(graph) {}

  DerivedT& BaseAddr(Node* base_addr) {
    base_addr_node_ = base_addr;
    return derived();
  }

  DerivedT& Offset(Node* offset) {
    offset_node_ = offset;
    return derived();
  }

 protected:
  Graph* graph_;
  Node *base_addr_node_, *offset_node_;
};

}  // namespace internal

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_NODEUTILSBASE_H