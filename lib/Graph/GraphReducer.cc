#include "graphir/Graph/GraphReducer.h"

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/properties.hpp>
#include <iostream>
#include <vector>

#include "graphir/Graph/BGL.h"
#include "graphir/Graph/NodeUtils.h"

namespace graphir {

struct GraphReducer::DFSVisitor : public boost::default_dfs_visitor {
  DFSVisitor(std::vector<Node*>& Preced,
             NodeMarker<GraphReducer::ReductionState>& M)
      : Precedence(Preced), Marker(M) {
    Precedence.clear();
  }

  void finish_vertex(Node* N, const SubGraph& G) {
    Precedence.push_back(N);
    Marker.set(N, GraphReducer::ReductionState::OnStack);
  }

 private:
  std::vector<Node*>& Precedence;
  NodeMarker<GraphReducer::ReductionState>& Marker;
};

GraphReducer::GraphReducer(Graph& graph, bool TrimGraph)
    : graph_(graph),
      dead_node_(NodeBuilder<IrOpcode::Dead>(&graph_).Build()),
      rs_marker_(graph_, 4),
      do_trim_graph_(TrimGraph) {}

void GraphReducer::Replace(Node* N, Node* Replacement) {
  for (auto* Usr : N->users()) {
    Revisit(Usr);
  }
  N->ReplaceWith(Replacement);
  N->kill(dead_node_);
  Recurse(Replacement);
}

void GraphReducer::Revisit(Node* N) {
  if (rs_marker_.get(N) == ReductionState::Visited) {
    rs_marker_.set(N, ReductionState::Revisit);
    revisit_stack_.insert(revisit_stack_.begin(), N);
  }
}

void GraphReducer::Push(Node* N) {
  rs_marker_.set(N, ReductionState::OnStack);
  reduction_stack_.insert(reduction_stack_.begin(), N);
}

void GraphReducer::Pop() {
  auto* TopNode = reduction_stack_.front();
  reduction_stack_.erase(reduction_stack_.begin());
  rs_marker_.set(TopNode, ReductionState::Visited);
}

bool GraphReducer::Recurse(Node* N) {
  if (rs_marker_.get(N) > ReductionState::Revisit)
    return false;
  Push(N);
  return true;
}

void GraphReducer::DFSVisit(SubGraph& SG, NodeMarker<ReductionState>& Marker) {
  DFSVisitor Vis(reduction_stack_, Marker);
  std::unordered_map<Node*, boost::default_color_type> ColorStorage;
  StubColorMap<decltype(ColorStorage), Node> ColorMap(ColorStorage);
  boost::depth_first_search(SG, Vis, std::move(ColorMap));
}

void GraphReducer::RunOnFunctionGraph(SubGraph& SG,
                                      detail::ReducerConcept* Reducer) {
  DFSVisit(SG, rs_marker_);

  while (!reduction_stack_.empty() || !revisit_stack_.empty()) {
    while (!reduction_stack_.empty()) {
      Node* N = reduction_stack_.front();
      if (N->getOp() == IrOpcode::Dead) {
        Pop();
        continue;
      }

      auto RP = Reducer->Reduce(N);

      if (!RP.Changed()) {
        Pop();
        continue;
      }

      if (RP.Replacement() == N) {
        // in-place replacement, recurse on input
        bool IsRecursed = false;
        for (auto* Input : N->inputs()) {
          if (Input != N)
            IsRecursed |= Recurse(Input);
        }
        if (IsRecursed)
          continue;
      }

      Pop();

      if (RP.Replacement() != N) {
        Replace(N, RP.Replacement());
      } else {
        // revisit all the users
        for (auto* Usr : N->users()) {
          if (Usr != N)
            Revisit(Usr);
        }
      }
    }

    while (!revisit_stack_.empty()) {
      Node* N = revisit_stack_.front();
      revisit_stack_.erase(revisit_stack_.begin());

      if (rs_marker_.get(N) == ReductionState::Revisit) {
        Push(N);
      }
    }
  }
}

void GraphReducer::RunImpl(detail::ReducerConcept* Reducer) {
  for (auto& SG : graph_.subregions())
    RunOnFunctionGraph(SG, Reducer);

  if (do_trim_graph_) {
    // remove nodes that are unreachable from any function
    NodeMarker<ReductionState> TrimMarker(graph_, 4);
    for (auto& SG : graph_.subregions()) {
      DFSVisit(SG, TrimMarker);
    }
    for (auto NI = graph_.node_begin(); NI != graph_.node_end();) {
      auto* N = Graph::GetNodeFromIt(NI);
      if (TrimMarker.get(N) == ReductionState::Unvisited &&
          !NodeProperties<IrOpcode::VirtGlobalValues>(N) && !graph_.IsGlobalVar(N)) {
        NI = graph_.RemoveNode(NI);
      } else {
        ++NI;
      }
    }

    // remove all deps to Dead node
    auto* DeadNode = NodeBuilder<IrOpcode::Dead>(&graph_).Build();
    std::vector<Node*> DeadUsrs;
    DeadUsrs.assign(DeadNode->value_users().begin(),
                    DeadNode->value_users().end());
    for (auto* N : DeadUsrs)
      N->removeValueInputAll(DeadNode);
    DeadUsrs.assign(DeadNode->effect_users().begin(),
                    DeadNode->effect_users().end());
    for (auto* N : DeadUsrs)
      N->removeEffectInputAll(DeadNode);
    DeadUsrs.assign(DeadNode->control_users().begin(),
                    DeadNode->control_users().end());
    for (auto* N : DeadUsrs)
      N->removeEffectInputAll(DeadNode);
  }
}

}  // namespace graphir