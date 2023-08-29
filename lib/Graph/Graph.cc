#include "graphir/Graph/Graph.h"

#include "graphir/Graph/BGL.h"

#include <algorithm>
#include <boost/graph/graphviz.hpp>
#include <iterator>

#include "graphir/Graph/NodeMarker.h"
#include "graphir/Graph/NodeUtils.h"

namespace graphir {

Graph::edge_iterator Graph::edge_begin() { return edge_iterator(this); }
Graph::edge_iterator Graph::edge_end() { return edge_iterator(this, true); }
size_t Graph::edge_size() { return std::distance(edge_begin(), edge_end()); }

SubGraph::edge_iterator SubGraph::edge_begin() { return edge_iterator(this); }
SubGraph::edge_iterator SubGraph::edge_end() {
  return edge_iterator(this, true);
}
size_t SubGraph::edge_size() { return std::distance(edge_begin(), edge_end()); }

void Graph::MarkGlobalVar(Node* N) {
  assert(N->getOp() == IrOpcode::SrcVarDecl ||
         N->getOp() == IrOpcode::SrcArrayDecl ||
         N->getOp() == IrOpcode::Alloca);
  global_variables_.insert(N);
}

void Graph::ReplaceGlobalVar(Node* old_node, Node* new_node) {
  if (IsGlobalVar(old_node)) {
    global_variables_.erase(old_node);
    MarkGlobalVar(new_node);
  }
}

void Graph::InsertNode(Node* N) {
  nodes_.emplace_back(N);
  if (node_idx_marker_) {
    node_idx_marker_->set(N, node_idx_counter_++);
  }
}

typename Graph::node_iterator Graph::RemoveNode(
    typename Graph::node_iterator it) {
  auto* N = it->get();
  if (!N->isDead()) {
    auto* dead_node = NodeBuilder<IrOpcode::Dead>(this).Build();
    N->kill(dead_node);
  }
  // unlink it with dead_node_
  N->removeValueInputAll(dead_node_);
  N->removeEffectInputAll(dead_node_);
  N->removeControlInputAll(dead_node_);
  return nodes_.erase(it);
}

void Graph::AddSubRegion(const SubGraph& sg) { sub_regions_.push_back(sg); }
void Graph::AddSubRegion(SubGraph&& sg) { sub_regions_.push_back(sg); }

void Graph::DumpGraphviz(std::ostream& os) {
  boost::write_graphviz(os, *this, graph_vertex_prop_writer(*this),
                        graph_edge_prop_writer{}, graph_prop_writer{});
}

size_t SubGraph::node_size() const {
  return std::distance(node_cbegin(), node_cend());
}

}  // namespace graphir