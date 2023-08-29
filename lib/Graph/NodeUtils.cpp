#include "graphir/Graph/NodeUtils.h"
#include <utility>
#include <vector>

using namespace graphir;

Node* graphir::FindNearestCtrlPoint(Node* N) {
  // perform a simple BFS to find the nearest control dependency
  // point
  std::vector<Node*> BFSQueue;
  BFSQueue.push_back(N);
  while(!BFSQueue.empty()) {
    auto* CurNode = BFSQueue.front();
    for(auto* N : CurNode->inputs()) {
      if(NodeProperties<IrOpcode::VirtCtrlPoints>(N))
        return N;
      else
        BFSQueue.push_back(N);
    }
    BFSQueue.erase(BFSQueue.begin());
  }
  return nullptr;
}
