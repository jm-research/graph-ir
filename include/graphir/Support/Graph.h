#ifndef GRAPHIR_SUPPORT_GRAPH_H
#define GRAPHIR_SUPPORT_GRAPH_H

#include <boost/graph/properties.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <set>
#include <vector>

#include "graphir/Graph/Node.h"
#include "graphir/Support/type_traits.h"

namespace graphir {

template <class GraphT, class PropertyTag>
struct graph_id_map {};

template <class GraphT>
class lazy_edge_iterator
    : public boost::iterator_facade<lazy_edge_iterator<GraphT>, Use,
                                    boost::forward_traversal_tag, Use> {
  friend class boost::iterator_core_access;
  GraphT* graph_;
  unsigned curr_input_;
  typename GraphT::node_iterator curr_node_it_, end_node_it_;

  inline Node* CurrNode() const { return GraphT::GetNodeFromIt(curr_node_it_); }

  inline bool isValid() const {
    return graph_ && curr_node_it_ != end_node_it_ &&
           curr_input_ < CurrNode()->inputs_.size();
  }

  bool equal(const lazy_edge_iterator& other) const {
    return graph_ == other.graph_ && curr_node_it_ == other.curr_node_it_ &&
           (curr_input_ == other.curr_input_ || curr_node_it_ == end_node_it_);
  }

  void nextValidPos() {
    while (curr_node_it_ != end_node_it_ &&
           curr_input_ >= CurrNode()->inputs_.size()) {
      ++curr_node_it_;
      curr_input_ = 0;
    }
  }

  void increment() {
    ++curr_input_;
    nextValidPos();
  }

  Use dereference() const {
    assert(isValid() && "can not dereference invalid iterator");
    auto dep_k = CurrNode()->inputUseKind(curr_input_);
    return Use(CurrNode(), CurrNode()->inputs_[curr_input_], dep_k);
  }

 public:
  lazy_edge_iterator() : graph_(nullptr), curr_input_(0) {}

  explicit lazy_edge_iterator(GraphT* graph, bool is_end = false)
      : graph_(graph), curr_input_(0) {
    if (graph_) {
      end_node_it_ = graph_->node_end();
      if (!is_end) {
        curr_node_it_ = graph_->node_begin();
        nextValidPos();
      } else {
        curr_node_it_ = end_node_it_;
      }
    }
  }
};

template <class GraphT, bool is_const,
          class NodeT = graphir::conditional_t<is_const, const Node*, Node*>>
class lazy_node_iterator
    : public boost::iterator_facade<lazy_node_iterator<GraphT, is_const, NodeT>,
                                    NodeT, boost::forward_traversal_tag,
                                    NodeT> {
  friend class boost::iterator_core_access;

  std::vector<NodeT> queue_;
  std::set<NodeT> visited_;

  void enqueue(NodeT n) {
    queue_.push_back(n);
    visited_.insert(n);
  }

  bool equal(const lazy_node_iterator& other) const {
    if (queue_.size() != other.queue_.size()) {
      return false;
    }
    if (queue_.empty()) {
      return true;
    }
    // deep compare
    for (auto I1 = queue_.cbegin(), I2 = other.queue_.cbegin(),
              E1 = queue_.cend(), E2 = other.queue_.cend();
         I1 != E1 && I2 != E2; ++I1, ++I2) {
      if (*I1 != *I2) {
        return false;
      }
    }
    return true;
  }

  NodeT dereference() const {
    assert(!queue_.empty());
    return queue_.front();
  }

  void increment() {
    NodeT top = queue_.front();
    queue_.erase(queue_.begin());
    for (NodeT node : top->inputs()) {
      if (visited_.count(node)) {
        continue;
      }
      enqueue(node);
    }
  }

 public:
  lazy_node_iterator() = default;
  explicit lazy_node_iterator(NodeT end_node) { enqueue(end_node); }
};

template <class T, class VertexTy>
struct StubColorMap {
  using value_type = boost::default_color_type;
  using reference = value_type&;
  using key_type = VertexTy*;
  struct category : public boost::read_write_property_map_tag {};

  StubColorMap(T& impl) : storage_(impl) {}
  StubColorMap() = delete;
  StubColorMap(const StubColorMap& other) = default;

  reference get(const key_type& key) const {
    return const_cast<reference>(storage_.at(key));
  }
  void put(const key_type& key, const value_type& val) { storage_[key] = val; }

 private:
  T& storage_;
};

template <class T, class Vertex>
inline typename graphir::StubColorMap<T, Vertex>::reference get(
    const graphir::StubColorMap<T, Vertex>& pmap,
    const typename graphir::StubColorMap<T, Vertex>::key_type& key) {
  return pmap.get(key);
}

template <class T, class Vertex>
inline void put(
    graphir::StubColorMap<T, Vertex>& pmap,
    const typename graphir::StubColorMap<T, Vertex>::key_type& key,
    const typename graphir::StubColorMap<T, Vertex>::value_type& val) {
  return pmap.put(key, val);
}

}  // namespace graphir

#endif  // GRAPHIR_SUPPORT_GRAPH_H