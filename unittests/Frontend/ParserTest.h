#ifndef GRAPHIR_FRONTEND_PARSERTEST_H
#define GRAPHIR_FRONTEND_PARSERTEST_H
/// Some utilities only for parser unittests
#include "graphir/Frontend/Parser.h"
#include "graphir/Graph/Graph.h"
#include "graphir/Graph/NodeUtils.h"

namespace graphir {
inline void SetMockContext(Parser& P, Graph& G) {
  P.NewSymScope();
  auto* DummyFunc = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
                    .FuncName("dummy_func")
                    .Build();
  P.setLastCtrlPoint(DummyFunc);
}
} // end namespace graphir
#endif
