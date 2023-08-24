#ifndef GRAPHIR_GRAPH_NODE_H
#define GRAPHIR_GRAPH_NODE_H

#include <boost/iterator/filter_iterator.hpp>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "graphir/Graph/Opcodes.h"
#include "graphir/Support/Hash.h"
#include "graphir/Support/Log.h"
#include "graphir/Support/iterator_range.h"

namespace graphir {

class Node;

namespace detail {

template <IrOpcode::ID OC, class SubT>
struct BinOpNodeBuilder;

}  // namespace detail

struct Use {
  enum Kind : uint8_t { K_NONE = 0, K_VALUE, K_CONTROL, K_EFFECT };

  Node* source;
  Node* dest;
  Kind dep_kind;

  Use() : source(nullptr), dest(nullptr), dep_kind(K_NONE) {}
  Use(Node* src, Node* dst, Kind kind = K_VALUE)
      : source(src), dest(dst), dep_kind(kind) {}

  bool operator==(const Use& rhs) const {
    return source == rhs.source && dest == rhs.dest && dep_kind == rhs.dep_kind;
  }
  bool operator!=(const Use& rhs) const { return !(rhs == *this); }

  struct BuilderFunctor;
};

struct Use::BuilderFunctor {
  Node* from;
  Use::Kind dep_kind;

  using PatcherTy = std::function<Use(const Use&)>;
  PatcherTy patch_functor;

  BuilderFunctor() = default;
  explicit BuilderFunctor(Node* node, Use::Kind kind = K_NONE,
                          PatcherTy patcher = nullptr)
      : from(node), dep_kind(kind), patch_functor(patcher) {}

  Use operator()(Node* to) const {
    Use e(from, to, dep_kind);
    if (patch_functor) {
      return patch_functor(e);
    } else {
      return e;
    }
  }
};

class Node {
  friend class Graph;

  template <class T>
  friend class lazy_edge_iterator;
  template <IrOpcode::ID>
  friend class NodePropertiesBase;
  template <IrOpcode::ID>
  friend class NodeProperties;
  friend class NodeMarkerBase;
  template <IrOpcode::ID>
  friend class NodeBuilder;
  template <IrOpcode::ID OC, class SubT>
  friend struct detail::BinOpNodeBuilder;

  IrOpcode::ID op_;

  uint32_t marker_data_;

  unsigned num_value_input_;
  unsigned num_control_input_;
  unsigned num_effect_input_;

  std::vector<Node*> inputs_;
  std::vector<Node*> users_;

  bool is_killed_;

  using input_iterator = typename decltype(inputs_)::iterator;
  using const_input_iterator = typename decltype(inputs_)::const_iterator;

  inline Use::Kind inputUseKind(unsigned raw_input_idx) {
    assert(raw_input_idx < inputs_.size());
    if (raw_input_idx < num_value_input_) {
      return Use::K_VALUE;
    } else if (raw_input_idx < num_value_input_ + num_control_input_) {
      return Use::K_CONTROL;
    } else if (raw_input_idx <
               num_value_input_ + num_control_input_ + num_effect_input_) {
      return Use::K_EFFECT;
    } else {
      return Use::K_NONE;
    }
  }

  void setNodeInput(unsigned index, unsigned size, unsigned offset,
                    Node* new_node);
  void appendNodeInput(unsigned& size, unsigned offset, Node* new_node);
  void removeNodeInput(unsigned index, unsigned& size, unsigned offset);
  void removeNodeInputAll(Node* node, unsigned& size, unsigned offset);
  void cleanupRemoveNodeInput(Node* old_input);

 public:
  using MarkerTy = uint32_t;

  IrOpcode::ID getOp() const { return op_; }

  inline unsigned getNumValueInput() const { return num_value_input_; }
  inline unsigned getNumControlInput() const { return num_control_input_; }
  inline unsigned getNumEffectInput() const { return num_effect_input_; }

  Node* getValueInput(unsigned index) const {
    assert(index < num_value_input_);
    return inputs_[index];
  }

  Node* getControlInput(unsigned index) const {
    assert(index < num_control_input_);
    return inputs_[num_value_input_ + index];
  }

  Node* getEffectInput(unsigned index) const {
    assert(index < num_effect_input_);
    return inputs_[num_value_input_ + num_control_input_ + index];
  }

  void setValueInput(unsigned idx, Node* new_node);
  void appendValueInput(Node* new_node);
  void removeValueInput(unsigned idx);
  void removeValueInputAll(Node* node);

  void setControlInput(unsigned idx, Node* new_node);
  void appendControlInput(Node* new_node);
  void removeControlInput(unsigned idx);
  void removeControlInputAll(Node* node);

  void setEffectInput(unsigned idx, Node* new_node);
  void appendEffectInput(Node* new_node);
  void removeEffectInput(unsigned idx);
  void removeEffectInputAll(Node* node);

  void kill(Node* dead_node);
  bool isDead() const { return is_killed_; }

  iterator_range<input_iterator> inputs() {
    return make_range(inputs_.begin(), inputs_.end());
  }

  iterator_range<const_input_iterator> inputs() const {
    return make_range(inputs_.begin(), inputs_.end());
  }

  input_iterator input_begin() { return inputs_.begin(); }
  input_iterator input_end() { return inputs_.end(); }
  size_t input_size() const { return inputs_.size(); }

  iterator_range<input_iterator> value_inputs() {
    return make_range(inputs_.begin(), inputs_.begin() + num_value_input_);
  }
  input_iterator value_input_begin() { return value_inputs().begin(); }
  input_iterator value_input_end() { return value_inputs().end(); }

  iterator_range<input_iterator> control_inputs() {
    return make_range(inputs_.begin() + num_value_input_,
                      inputs_.begin() + num_value_input_ + num_control_input_);
  }
  input_iterator control_input_begin() { return control_inputs().begin(); }
  input_iterator control_input_end() { return control_inputs().end(); }

  iterator_range<input_iterator> effect_inputs() {
    return make_range(inputs_.begin() + num_value_input_ + num_control_input_,
                      inputs_.begin() + num_value_input_ + num_control_input_ +
                          num_effect_input_);
  }
  input_iterator effect_input_begin() { return effect_inputs().begin(); }
  input_iterator effect_input_end() { return effect_inputs().end(); }

  struct is_value_use {
    Node* src_node;
    is_value_use(Node* src) : src_node(src) {}

    bool operator()(Node* sink) {
      for (Node* dep : sink->value_inputs()) {
        if (dep == src_node) {
          return true;
        }
      }
      return false;
    }
  };

  struct is_control_use {
    Node* src_node;
    is_control_use(Node* src) : src_node(src) {}

    bool operator()(Node* sink) {
      for (Node* dep : sink->control_inputs()) {
        if (dep == src_node) {
          return true;
        }
      }
      return false;
    }
  };

  struct is_effect_use {
    Node* src_node;
    is_effect_use(Node* src) : src_node(src) {}

    bool operator()(Node* sink) {
      for (Node* dep : sink->effect_inputs()) {
        if (dep == src_node) {
          return true;
        }
      }
      return false;
    }
  };

  using user_iterator = typename decltype(users_)::iterator;
  using value_user_iterator =
      boost::filter_iterator<is_value_use, user_iterator>;
  using control_user_iterator =
      boost::filter_iterator<is_control_use, user_iterator>;
  using effect_user_iterator =
      boost::filter_iterator<is_effect_use, user_iterator>;

  iterator_range<user_iterator> users() {
    return make_range(users_.begin(), users_.end());
  }
  iterator_range<value_user_iterator> value_users();
  iterator_range<control_user_iterator> control_users();
  iterator_range<effect_user_iterator> effect_users();

  size_t user_size() const { return users_.size(); }

  Node()
      : op_(IrOpcode::None),
        marker_data_(0),
        num_value_input_(0),
        num_control_input_(0),
        num_effect_input_(0),
        is_killed_(false) {}

  Node(IrOpcode::ID oc)
      : op_(oc),
        marker_data_(0),
        num_value_input_(0),
        num_control_input_(0),
        num_effect_input_(0),
        is_killed_(false) {}

  Node(IrOpcode::ID oc, const std::vector<Node*>& values,
       const std::vector<Node*>& controls = {},
       const std::vector<Node*>& effects = {});

  bool ReplaceUseOfWith(Node* from, Node* to, Use::Kind use_kind);
  bool ReplaceWith(Node* replacement, Use::Kind use_kind = Use::K_VALUE);
};

template <typename ValueT>
class NodeBiMap {
  std::unordered_map<Node*, ValueT> node_to_value_;
  using N2VPairTy = typename decltype(node_to_value_)::value_type;
  std::unordered_map<ValueT, Node*> value_to_node_;

 public:
  ValueT* find_value(Node* node) const {
    return node_to_value_.count(node)
               ? const_cast<ValueT*>(&node_to_value_.at(node))
               : nullptr;
  }

  Node* find_node(ValueT value) const {
    return value_to_node_.count(value)
               ? const_cast<Node*>(value_to_node_.at(value))
               : nullptr;
  }

  bool insert(const N2VPairTy& pair, bool overwrite = false) {
    if (overwrite) {
      node_to_value_[pair.first] = pair.second;
      value_to_node_[pair.second] = pair.first;
    } else {
      if (node_to_value_.count(pair.first) ||
          value_to_node_.count(pair.second)) {
        return false;
      }
      node_to_value_.insert(pair);
      value_to_node_.insert(std::make_pair(pair.second, pair.first));
    }
    return true;
  }

  void erase(Node* node) {
    if (node_to_value_.count(node)) {
      value_to_node_.erase(node_to_value_.at(node));
      node_to_value_.erase(node);
    }
  }

  size_t size() const { return value_to_node_.size(); }
};

}  // namespace graphir

namespace std {

template <>
struct hash<graphir::Use> {
  using arg_type = graphir::Use;
  using result_type = size_t;
  size_t operator()(const graphir::Use& use) const {
    result_type seed = 0;
    return hash_combine(seed, use.source) ^ hash_combine(seed, use.dest) ^
           hash_combine(seed, use.dep_kind);
  }
};

}  // namespace std

#endif  // GRAPHIR_GRAPH_NODE_H