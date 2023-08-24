#include "graphir/Graph/AttributeBuilder.h"

#include "graphir/Graph/Graph.h"

namespace graphir {

// will clear the Attrs buffer in the current builder
void AttributeBuilder::Attach(Node* n) {
  auto& attr_list = graph_.attributes_[n];
  attr_list.splice(attr_list.end(), std::move(attrs_));
  attr_set_.clear();
}

}  // namespace graphir