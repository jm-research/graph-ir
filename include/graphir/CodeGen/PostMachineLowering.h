#ifndef GRAPHIR_CODEGEN_POSTMACHINELOWERING_H
#define GRAPHIR_CODEGEN_POSTMACHINELOWERING_H
#include "graphir/CodeGen/GraphScheduling.h"

namespace graphir {
class PostMachineLowering {
  GraphSchedule& Schedule;
  Graph& G;

  void ControlFlowLowering();

  void FunctionCallLowering();

  void Trimming();

public:
  PostMachineLowering(GraphSchedule& schedule);

  void Run();
};
} // end namespace graphir
#endif
