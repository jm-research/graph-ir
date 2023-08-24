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

struct GraphEditor {
  struct Interface {
    virtual ~Interface() = default;

    virtual void Replace(Node* node, Node* replacement) = 0;
    virtual void Revisit(Node* node) = 0;

    virtual Graph& GetGraph() = 0;
  };

 protected:
  explicit GraphEditor(Interface* editor) : editor_(editor) {}

  Graph& GetGraph() { return editor_->GetGraph(); }

  void Replace(Node* node, Node* replacement) {
    editor_->Replace(node, replacement);
  }

  void Revisit(Node* node) { editor_->Revisit(node); }

  static GraphReduction Replace(Node* node) { return GraphReduction(node); }

  static GraphReduction NoChange() { return GraphReduction(); }

  Interface* editor_;
};

class GraphReducer : public GraphEditor::Interface {
  enum class ReductionState : uint8_t {
    Unvisited = 0,  // Default state
    Revisit,        // Revisit later
    OnStack,        // Observed and on stack
    Visited         // Finished
  };

  struct DFSVisitor;

  Graph& graph_;
  Node* dead_node_;

  std::vector<Node*> reduction_stack_, revisit_stack_;
  NodeMarker<ReductionState> rs_marker_;

  bool do_trim_graph_;

  GraphReducer(Graph& graph, bool trim_graph = true);

  void Replace(Node* node, Node* replacement) override;
  void Revisit(Node* node) override;
  Graph& GetGraph() override { return graph_; }

  void Push(Node* node);
  void Pop();
  bool Recurse(Node* node);

  void DFSVisit(SubGraph& sg, NodeMarker<ReductionState>& marker);

  void RunImpl(detail::ReducerConcept* reducer);
  void RunOnFunctionGraph(SubGraph& sg, detail::ReducerConcept* reducer);

 public:
  template <class ReducerT, class... Args>
  static void Run(Graph& graph, Args&&... ctor_args) {
    GraphReducer gr(graph);
    detail::ReducerModel<ReducerT, Args...> rm(
        std::forward<Args>(ctor_args)...);
    gr.RunImpl(&rm);
  }

  template <class ReducerT, class... Args>
  static void RunWithEditor(Graph& graph, Args&&... ctor_args) {
    GraphReducer gr(graph);
    detail::ReducerModel<ReducerT, GraphEditor::Interface*, Args...> rm(
        &gr, std::forward<Args>(ctor_args)...);
    gr.RunImpl(&rm);
  }
};

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_GRAPHREDUCER_H