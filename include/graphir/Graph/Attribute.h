#ifndef GRAPHIR_GRAPH_ATTRIBUTE_H
#define GRAPHIR_GRAPH_ATTRIBUTE_H

#include "graphir/Graph/Node.h"

namespace graphir {

enum class Attr {
  NoMem,
  ReadMem,
  WriteMem,
  HasSideEffect,  // environment side-effects
  IsBuiltin
};

template <Attr AT>
class Attribute;

struct AttributeConcept {
  template <Attr AT>
  Attribute<AT>& as() {
    assert(Kind() == AT && "invalid attr kind");
    return *static_cast<Attribute<AT>*>(this);
  }

  template <Attr AT>
  const Attribute<AT>& as() const {
    assert(Kind() == AT && "invalid attr kind");
    return *static_cast<const Attribute<AT>*>(this);
  }

  virtual Attr Kind() const = 0;

  virtual ~AttributeConcept() = default;
};

template <Attr AT>
class Attribute : public AttributeConcept {
 public:
  Attr Kind() const override { graphir_unreachable("unimplemented"); }
};

#define ATTRIBUTE_IMPL(AT) \
  template <>              \
  class Attribute<Attr::AT> : public AttributeConcept

// clang-format off
ATTRIBUTE_IMPL(NoMem) {
  Attr Kind() const override { return Attr::NoMem; }
};

ATTRIBUTE_IMPL(ReadMem) {
  Attr Kind() const override { return Attr::ReadMem; }
};

ATTRIBUTE_IMPL(WriteMem) {
  Attr Kind() const override { return Attr::WriteMem; }
};

ATTRIBUTE_IMPL(HasSideEffect) {
  Attr Kind() const override { return Attr::HasSideEffect; }
};

ATTRIBUTE_IMPL(IsBuiltin) {
  Attr Kind() const override { return Attr::IsBuiltin; }
};

#undef ATTRIBUTE_IMPL
// clang-format on

}  // namespace graphir

#endif  // GRAPHIR_GRAPH_ATTRIBUTE_H