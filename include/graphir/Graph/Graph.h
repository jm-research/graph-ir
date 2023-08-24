#ifndef GRAPHIR_GRAPH_GRAPH_H
#define GRAPHIR_GRAPH_GRAPH_H

#include <iostream>
#include <list>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "graphir/Graph/Attribute.h"
#include "graphir/Graph/Node.h"
#include "graphir/Support/Graph.h"
#include "graphir/Support/iterator_range.h"

namespace graphir {

class SubGraph {
  template <class T>
  friend struct std::hash;

  Node* tail_node_;

  typename Use::BuilderFunctor::PatcherTy edge_patcher_;

 public:
  using node_iterator = lazy_node_iterator<SubGraph, false>;
  using const_node_iterator = lazy_node_iterator<SubGraph, true>;
  using edge_iterator = lazy_edge_iterator<SubGraph>;

  SubGraph() : tail_node_(nullptr), edge_patcher_(nullptr) {}
  explicit SubGraph(Node* tail) : tail_node_(tail) {}

  bool operator==(const SubGraph& other) const {
    return tail_node_ == other.tail_node_;
  }

  void SetEdgePatcher(Use::BuilderFunctor::PatcherTy patcher) {
    edge_patcher_ = patcher;
  }
  void ClearEdgePatcher() { edge_patcher_ = nullptr; }
  const Use::BuilderFunctor::PatcherTy& GetEdgePatcher() const {
    return edge_patcher_;
  }

  static Node* GetNodeFromIt(const node_iterator& it) { return *it; }
  static const Node* GetNodeFromIt(const const_node_iterator& it) {
    return *it;
  }
  node_iterator node_begin() { return node_iterator(tail_node_); }
  node_iterator node_end() { return node_iterator(); }
  iterator_range<node_iterator> nodes() {
    return make_range(node_begin(), node_end());
  }
  const_node_iterator node_cbegin() const {
    return const_node_iterator(const_cast<const Node*>(tail_node_));
  }
  const_node_iterator node_cend() const { return const_node_iterator(); }
  size_t node_size() const;

  edge_iterator edge_begin();
  edge_iterator edge_end();
  size_t edge_size();
  size_t edge_size() const { return const_cast<SubGraph*>(this)->edge_size(); }
};

}  // namespace graphir

namespace std {

template <>
struct hash<graphir::SubGraph> {
  using argument_type = graphir::SubGraph;
  using result_type = std::size_t;
  result_type operator()(const argument_type& sg) const noexcept {
    return std::hash<graphir::Node*>{}(sg.tail_node_);
  }
};

}  // namespace std

namespace graphir {

struct AttributeBuilder;
template <class T>
struct NodeMarker;

// Owner of nodes
class Graph {
  template <IrOpcode::ID Op>
  friend class NodeBuilder;
  template <IrOpcode::ID Op>
  friend struct NodeProperties;
  friend class NodeMarkerBase;
  friend struct AttributeBuilder;

  std::vector<std::unique_ptr<Node>> nodes_;

  NodeBiMap<std::string> const_str_pool_;
  NodeBiMap<int32_t> const_number_pool_;
  Node* dead_node_;

  std::vector<SubGraph> sub_regions_;
  NodeBiMap<SubGraph> func_stub_pool_;

  using AttributeList = std::list<std::unique_ptr<AttributeConcept>>;
  std::unordered_map<Node*, AttributeList> attributes_;
  std::unordered_set<Node*> global_variables_;

  typename Node::MarkerTy marker_max_;
  typename Use::BuilderFunctor::PatcherTy edge_patcher_;

  NodeMarker<uint16_t>* node_idx_marker_;
  uint16_t node_idx_counter_;

 public:
  using node_iterator = typename decltype(nodes_)::iterator;
  using const_node_iterator = typename decltype(nodes_)::const_iterator;
  using edge_iterator = lazy_edge_iterator<Graph>;
  using subregion_iterator = typename decltype(sub_regions_)::iterator;
  using global_var_iterator = typename decltype(global_variables_)::iterator;

  Graph()
      : dead_node_(nullptr),
        marker_max_(0),
        edge_patcher_(nullptr),
        node_idx_marker_(nullptr),
        node_idx_counter_(0) {}

  void SetEdgePatcher(Use::BuilderFunctor::PatcherTy patcher) {
    edge_patcher_ = patcher;
  }
  void ClearEdgePatcher() { edge_patcher_ = nullptr; }
  const Use::BuilderFunctor::PatcherTy& GetEdgePatcher() const {
    return edge_patcher_;
  }

  void SetNodeIdxMarker(NodeMarker<uint16_t>* marker) {
    node_idx_marker_ = marker;
    node_idx_counter_ = 0;
  }
  void ClearNodeIdxMarker() { node_idx_marker_ = nullptr; }

  static Node* GetNodeFromIt(const node_iterator& it) { return it->get(); }
  static const Node* GetNodeFromIt(const const_node_iterator& it) {
    return it->get();
  }
  node_iterator node_begin() { return nodes_.begin(); }
  node_iterator node_end() { return nodes_.end(); }
  const_node_iterator node_cbegin() const { return nodes_.cbegin(); }
  const_node_iterator node_cend() const { return nodes_.cend(); }
  Node* getNode(size_t idx) { return nodes_[idx].get(); }
  size_t node_size() const { return nodes_.size(); }

  edge_iterator edge_begin();
  edge_iterator edge_end();
  size_t edge_size();
  size_t edge_size() const { return const_cast<Graph*>(this)->edge_size(); }

  iterator_range<subregion_iterator> subregions() {
    return make_range(sub_regions_.begin(), sub_regions_.end());
  }

  void InsertNode(Node* node);
  node_iterator RemoveNode(node_iterator it);

  void MarkGlobalVar(Node* node);
  bool IsGlobalVar(Node* node) const { return global_variables_.count(node); }
  void ReplaceGlobalVar(Node* old_node, Node* new_node);

  global_var_iterator global_var_begin() { return global_variables_.begin(); }
  global_var_iterator global_var_end() { return global_variables_.end(); }
  iterator_range<global_var_iterator> global_vars() {
    return make_range(global_var_begin(), global_var_end());
  }

  void AddSubRegion(SubGraph&& sg);
  void AddSubRegion(const SubGraph& sg);

  size_t GetNumConstStr() const { return const_str_pool_.size(); }

  size_t GetNumConstNumber() const { return const_number_pool_.size(); }

  void DumpGraphviz(std::ostream& os);
};

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_GRAPH_H