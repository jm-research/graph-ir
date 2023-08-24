#ifndef GRAPHIR_GRAPH_OPTIMIZER_PEEPHOLE_H
#define GRAPHIR_GRAPH_OPTIMIZER_PEEPHOLE_H

#include "graphir/Graph/GraphReducer.h"

namespace graphir {

class PeepholeReducer : public GraphEditor {
  Graph& graph_;
  Node* dead_node_;

  GraphReduction ReduceArithmetic(Node* node);
  GraphReduction ReduceRelation(Node* node);
  GraphReduction DeadPhiElimination(Node* node);

 public:
  PeepholeReducer(GraphEditor::Interface* editor);

  static constexpr const char* name() { return "Peephole"; }

  GraphReduction Reduce(Node* node);
};

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_OPTIMIZER_PEEPHOLE_H