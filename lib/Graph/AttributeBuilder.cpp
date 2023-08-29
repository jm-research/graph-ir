#include "graphir/Graph/AttributeBuilder.h"
#include "graphir/Graph/Graph.h"

using namespace graphir;

// will clear the Attrs buffer in the current builder
void AttributeBuilder::Attach(Node* N) {
  auto& AttrList = G.Attributes[N];
  AttrList.splice(AttrList.end(), std::move(Attrs));
  AttrSet.clear();
}
