#ifndef GRAPHIR_GRAPH_ATTRIBUTEBUILDER_H
#define GRAPHIR_GRAPH_ATTRIBUTEBUILDER_H

#include <list>
#include <memory>
#include <set>
#include <utility>

#include "graphir/Graph/Attribute.h"

namespace graphir {

class Graph;

struct AttributeBuilder {
  AttributeBuilder(Graph& graph) : graph_(graph) {}

  template <Attr AT, class... CtorArgs>
  AttributeBuilder& Add(CtorArgs&&... args) {
    attrs_.emplace_back(new Attribute<AT>(std::forward<CtorArgs>(args)...));
    attr_set_.insert(AT);
    return *this;
  }

  template <Attr AT>
  bool hasAttr() const {
    return attr_set_.count(AT);
  }

  void Attach(Node* node);

  bool empty() const { return attrs_.empty(); }

 private:
  Graph& graph_;
  std::list<std::unique_ptr<AttributeConcept>> attrs_;
  std::set<Attr> attr_set_;
};

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_ATTRIBUTEBUILDER_H