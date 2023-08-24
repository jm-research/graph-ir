#include "graphir/Graph/Node.h"

#include <iterator>
#include <utility>

#include "graphir/Support/STLExtras.h"

namespace graphir {

Node::Node(IrOpcode::ID oc, const std::vector<Node*>& values,
           const std::vector<Node*>& controls,
           const std::vector<Node*>& effects)
    : op_(oc),
      marker_data_(0),
      num_value_input_(values.size()),
      num_control_input_(controls.size()),
      num_effect_input_(effects.size()),
      is_killed_(false) {
  if (num_effect_input_ > 0) {
    inputs_.insert(inputs_.begin(), effects.begin(), effects.end());
  }
  if (num_control_input_ > 0) {
    inputs_.insert(inputs_.begin(), controls.begin(), controls.end());
  }
  if (num_value_input_ > 0) {
    inputs_.insert(inputs_.begin(), values.begin(), values.end());
  }
}

void Node::appendNodeInput(unsigned& size, unsigned offset, Node* new_node) {
  auto it = inputs_.begin();
  std::advance(it, size + offset);
  inputs_.insert(it, new_node);
  ++size;
  new_node->users_.push_back(this);
}

void Node::setNodeInput(unsigned index, unsigned size, unsigned offset,
                        Node* new_node) {
  index += offset;
  size += offset;
  assert(index < size);
  Node* old_node = inputs_[index];
  auto it = std::find(old_node->users_.begin(), old_node->users_.end(), this);
  assert(it != old_node->users_.end());
  old_node->users_.erase(it);
  inputs_[index] = new_node;
  new_node->users_.push_back(this);
}

void Node::cleanupRemoveNodeInput(Node* old_input) {
  auto it =
      std::find(old_input->users_.begin(), old_input->users_.end(), *this);
  assert(it != old_input->users_.end());
  old_input->users_.erase(it);
}

void Node::removeNodeInput(unsigned index, unsigned& size, unsigned offset) {
  unsigned s = size;
  index += offset;
  size += offset;
  assert(index < s);
  Node* old_node = inputs_[index];
  cleanupRemoveNodeInput(old_node);
  inputs_.erase(inputs_.begin() + index);
  --size;
}

void Node::removeNodeInputAll(Node* target, unsigned& size, unsigned offset) {
  auto i = inputs_.begin() + offset;
  while (i != (inputs_.begin() + size + offset)) {
    auto* n = *i;
    if (n == target) {
      cleanupRemoveNodeInput(n);
      i = inputs_.erase(i);
      --size;
    } else {
      ++i;
    }
  }
}

void Node::setValueInput(unsigned idx, Node* new_node) {
  setNodeInput(idx, num_value_input_, 0, new_node);
}

void Node::appendValueInput(Node* new_node) {
  appendNodeInput(num_value_input_, 0, new_node);
}

void Node::removeValueInput(unsigned idx) {
  removeNodeInput(idx, num_value_input_, 0);
}

void Node::removeValueInputAll(Node* node) {
  removeNodeInputAll(node, num_value_input_, 0);
}

void Node::setControlInput(unsigned idx, Node* new_node) {
  setNodeInput(idx, num_control_input_, num_value_input_, new_node);
}

void Node::appendControlInput(Node* new_node) {
  appendNodeInput(num_control_input_, num_value_input_, new_node);
}

void Node::removeControlInput(unsigned idx) {
  removeNodeInput(idx, num_control_input_, num_value_input_);
}

void Node::removeControlInputAll(Node* node) {
  removeNodeInputAll(node, num_control_input_, num_value_input_);
}

void Node::setEffectInput(unsigned idx, Node* new_node) {
  setNodeInput(idx, num_effect_input_, num_value_input_ + num_control_input_,
               new_node);
}

void Node::appendEffectInput(Node* new_node) {
  appendNodeInput(num_effect_input_, num_value_input_ + num_control_input_,
                  new_node);
}

void Node::removeEffectInput(unsigned idx) {
  removeNodeInput(idx, num_effect_input_,
                  num_value_input_ + num_control_input_);
}

void Node::removeEffectInputAll(Node* node) {
  removeNodeInputAll(node, num_effect_input_,
                     num_value_input_ + num_control_input_);
}

void Node::kill(Node* dead_node) {
  // remove inputs
  for (auto i = 0U, N = num_value_input_; i < N; ++i)
    setValueInput(i, dead_node);
  for (auto i = 0U, N = num_control_input_; i < N; ++i)
    setControlInput(i, dead_node);
  for (auto i = 0U, N = num_effect_input_; i < N; ++i)
    setEffectInput(i, dead_node);

  // replace all uses with Dead node
  ReplaceWith(dead_node, Use::K_VALUE);
  ReplaceWith(dead_node, Use::K_CONTROL);
  ReplaceWith(dead_node, Use::K_EFFECT);

  is_killed_ = true;
}

iterator_range<Node::value_user_iterator> Node::value_users() {
  is_value_use Pred(this);
  value_user_iterator it_begin(Pred, users_.begin(), users_.end());
  value_user_iterator it_end(Pred, users_.end(), users_.end());
  return make_range(it_begin, it_end);
}
iterator_range<Node::control_user_iterator> Node::control_users() {
  is_control_use Pred(this);
  control_user_iterator it_begin(Pred, users_.begin(), users_.end());
  control_user_iterator it_end(Pred, users_.end(), users_.end());
  return make_range(it_begin, it_end);
}
iterator_range<Node::effect_user_iterator> Node::effect_users() {
  is_effect_use Pred(this);
  effect_user_iterator it_begin(Pred, users_.begin(), users_.end());
  effect_user_iterator it_end(Pred, users_.end(), users_.end());
  return make_range(it_begin, it_end);
}

bool Node::ReplaceUseOfWith(Node* from, Node* to, Use::Kind use_kind) {
  switch (use_kind) {
    case Use::K_NONE:
      graphir_unreachable("Invalid Use Kind");
      break;
    case Use::K_VALUE: {
      auto It = graphir::find(value_inputs(), from);
      if (It == value_inputs().end())
        return false;
      auto Idx = std::distance(value_inputs().begin(), It);
      setValueInput(Idx, to);
      break;
    }
    case Use::K_CONTROL: {
      auto It = graphir::find(control_inputs(), from);
      if (It == control_inputs().end())
        return false;
      auto Idx = std::distance(control_inputs().begin(), It);
      setControlInput(Idx, to);
      break;
    }
    case Use::K_EFFECT: {
      auto It = graphir::find(effect_inputs(), from);
      if (It == effect_inputs().end())
        return false;
      auto Idx = std::distance(effect_inputs().begin(), It);
      setEffectInput(Idx, to);
      break;
    }
  }
  return true;
}

void Node::ReplaceWith(Node* replacement, Use::Kind use_kind) {
  switch (use_kind) {
    case Use::K_NONE:
      // replace every category of uses
      ReplaceWith(replacement, Use::K_VALUE);
      ReplaceWith(replacement, Use::K_CONTROL);
      ReplaceWith(replacement, Use::K_EFFECT);
      break;
    case Use::K_VALUE: {
      std::vector<Node*> Usrs(value_users().begin(), value_users().end());
      for (Node* Usr : Usrs) {
        Usr->ReplaceUseOfWith(this, replacement, use_kind);
      }
      break;
    }
    case Use::K_CONTROL: {
      std::vector<Node*> Usrs(control_users().begin(), control_users().end());
      for (Node* Usr : Usrs) {
        Usr->ReplaceUseOfWith(this, replacement, use_kind);
      }
      break;
    }
    case Use::K_EFFECT: {
      std::vector<Node*> Usrs(effect_users().begin(), effect_users().end());
      for (Node* Usr : Usrs) {
        Usr->ReplaceUseOfWith(this, replacement, use_kind);
      }
      break;
    }
  }
}

}  // namespace graphir