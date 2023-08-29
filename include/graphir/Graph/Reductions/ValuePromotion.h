#ifndef GRAPHIR_GRAPH_REDUCTIONS_VALUE_PROMOTION_H
#define GRAPHIR_GRAPH_REDUCTIONS_VALUE_PROMOTION_H
#include "graphir/Graph/GraphReducer.h"

namespace graphir {
class ValuePromotion : public GraphEditor {
  GraphReduction ReduceAssignment(Node* N);
  GraphReduction ReduceMemAssignment(Node* N);
  GraphReduction ReduceVarAccess(Node* N);
  GraphReduction ReduceArrayDecl(Node* N);
  GraphReduction ReduceMemAccess(Node* N);
  GraphReduction ReducePhiNode(Node* N);

  void appendValueUsage(Node* Usr, Node* Src, Node* Val);

  Graph& G;
  Node* DeadNode;

public:
  explicit ValuePromotion(GraphEditor::Interface* editor);

  static constexpr
  const char* name() { return "value-promotion"; }

  GraphReduction Reduce(Node* N);
};
} // end namespace graphir
#endif
