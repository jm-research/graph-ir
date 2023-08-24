#ifndef GRAPHIR_GRAPH_OPTIMIZER_CSE_H
#define GRAPHIR_GRAPH_OPTIMIZER_CSE_H

#include <set>
#include <unordered_map>

#include "graphir/Graph/GraphReducer.h"
#include "graphir/Graph/Node.h"

namespace graphir {

class CSEReducer : public GraphEditor {
  Graph& graph_;

  using node_hash_type = size_t;

  std::unordered_map<unsigned, std::set<Node*>> node_op_map_;
  NodeBiMap<node_hash_type> node_hash_map_;

  node_hash_type GetNodeHash(Node* node);

  void RevisitNodes(unsigned oc, Node* except);

  GraphReduction ReduceArithmetic(Node* node);
  GraphReduction ReduceMemoryLoad(Node* node);

 public:
  static constexpr const char* name() { return "CSE"; }

  CSEReducer(GraphEditor::Interface* editor);

  GraphReduction Reduce(Node* node);
};

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_OPTIMIZER_CSE_H