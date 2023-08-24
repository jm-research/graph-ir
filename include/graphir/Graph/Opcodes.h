#ifndef GRAPHIR_GRAPH_OPCODES_H
#define GRAPHIR_GRAPH_OPCODES_H

#include <iostream>

namespace graphir {

class Graph;
class Node;

namespace IrOpcode {

enum ID : unsigned {
  None = 0,
/// Frontend and middle-end opcodes
#define COMMON_OP(OC)    OC,
#define CONST_OP(OC)     OC,
#define CONTROL_OP(OC)   OC,
#define MEMORY_OP(OC)    OC,
#define INTERPROC_OP(OC) OC,
#define SRC_OP(OC)       Src##OC,
#include "graphir/Graph/Opcodes.def"

/// Machine level opcodes
#define DLX_ARITH_OP(OC) DLX##OC, DLX##OC##I,
#define DLX_COMMON(OC)   DLX##OC,
#define DLX_CONST(OC)    DLX_COMMON(OC)
#include "graphir/Graph/DLXOpcodes.def"

/// Virtual opcodes: abtraction nodes for several
/// opcodes with similar properties
#define VIRT_OP(OC) Virt##OC,
#include "graphir/Graph/Opcodes.def"
#define VIRT_OP(OC) Virt##OC,
#include "graphir/Graph/DLXOpcodes.def"
};

std::ostream& Print(const Graph& graph, std::ostream& os, Node* node);

}  // namespace IrOpcode
}  // namespace graphir

#endif  // GRAPHIR_GRAPH_OPCODES_H