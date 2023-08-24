#ifndef GRAPHIR_SUPPORT_AFFINECONTAINER_H
#define GRAPHIR_SUPPORT_AFFINECONTAINER_H

#include <array>
#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace graphir {

template <class ContainerTy, size_t Affinity = 2>
struct AffineContainer {
  using value_type = ContainerTy;

 protected:
  std::set<std::unique_ptr<ContainerTy>> repo_;

  struct Scope {
    size_t parent_branch, current_branch;
    std::array<ContainerTy*, Affinity> branches;

    Scope() : parent_branch(0), current_branch(0) {}
    Scope(ContainerTy* init, size_t parent_br)
        : parent_branch(parent_br), current_branch(0) {
      branches[0] = init;
    }

    size_t CurrBranch() const { return current_branch; }

    void AddBranch(ContainerTy* container) {
      size_t new_br = CurrBranch() + 1;
      assert(new_br < Affinity && "Exceeding Affinity");
      branches[new_br] = container;
      ++current_branch;
    }
  };

  std::list<Scope> scope_stack_;
  using scope_iterator = typename decltype(scope_stack_)::iterator;
  scope_iterator curr_scope_, prev_scope_;

  ContainerTy* PrevEntry() {
    if (prev_scope_ == scope_stack_.end() ||
        curr_scope_ == scope_stack_.end()) {
      return nullptr;
    }

    auto parent_br = curr_scope_->parent_branch;
    return prev_scope_->branches[parent_br];
  }

  void CopyOnWrite() {
    if (PrevEntry() && CurrEntry() == PrevEntry()) {
      const auto& orig = *PrevEntry();
      std::unique_ptr<ContainerTy> ptr(new ContainerTy(orig));
      curr_scope_->Branches[curr_scope_->CurrBranch()] = ptr.get();
      repo_.insert(std::move(ptr));
    }
  }

 public:
  AffineContainer() {
    std::unique_ptr<ContainerTy> table_ptr(new ContainerTy());
    scope_stack_.emplace_back(Scope(table_ptr.get(), 0));
    repo_.insert(std::move(table_ptr));

    curr_scope_ = scope_stack_.begin();
    prev_scope_ = scope_stack_.end();
  }

  size_t num_scopes() const { return scope_stack_.size(); }

  size_t num_entries() const { return curr_scope_->CurrBranch() + 1; }

  ContainerTy* CurrEntry() {
    if (curr_scope_ == scope_stack_.end()) {
      return nullptr;
    }
    return curr_scope_->branches[curr_scope_->CurrBranch()];
  }

  ContainerTy* CurrEntryMutable() {
    CopyOnWrite();
    return CurrEntry();
  }

  void NewAffineScope() {
    size_t curr_br = curr_scope_->CurrBranch();
    auto* curr_table = curr_scope_->branches[curr_br];
    prev_scope_ = curr_scope_;
    curr_scope_ =
        scope_stack_.insert(scope_stack_.end(), Scope(curr_table, curr_br));
  }

  void NewBranch() { curr_scope_->AddBranch(PrevEntry()); }

  template <
      class T = AffineContainer<ContainerTy, Affinity>,
      class Func = std::function<void(T&, const std::vector<ContainerTy*>&)>>
  void CloseAffineScope(Func callback) {
    assert(prev_scope_ != scope_stack_.end() && "Cannot close the root scope");
    auto& curr_entries = curr_scope_->branches;
    std::vector<ContainerTy*> entries(
        curr_entries.begin(),
        curr_entries.begin() + curr_scope_->CurrBranch() + 1);
    curr_scope_ = prev_scope_;
    if (prev_scope_ == scope_stack_.end()) {
      prev_scope_ = scope_stack_.end();
    } else {
      prev_scope_--;
    }
    callback(*static_cast<T*>(this), entries);
    scope_stack_.pop_back();
  }
};

template <class K, class V, size_t Affinity = 2>
class AffineRecordTable
    : public AffineContainer<std::unordered_map<K, V>, Affinity> {
  using BaseTy = AffineContainer<std::unordered_map<K, V>, Affinity>;

 public:
  using TableTy = std::unordered_map<K, V>;
  using table_iterator = typename TableTy::iterator;

  AffineRecordTable() = default;

  size_t num_tables() const { return BaseTy::num_entries(); }

  table_iterator begin() { return BaseTy::CurEntry()->begin(); }
  table_iterator end() { return BaseTy::CurEntry()->end(); }
  // find in current table
  table_iterator find(const K& key) { return BaseTy::CurEntry()->find(key); }
  size_t count(const K& key) { return BaseTy::CurEntry()->count(key); }

  V& operator[](const K& key) { return (*BaseTy::CurEntryMutable())[key]; }
  V& at(const K& key) { return (*BaseTy::CurEntryMutable()).at(key); }

  template <class Func = std::function<void(AffineRecordTable<K, V, Affinity>&,
                                            const std::vector<TableTy*>&)>>
  void CloseAffineScope(Func callback) {
    BaseTy::template CloseAffineScope<AffineRecordTable<K, V, Affinity>, Func>(
        callback);
  }
};

}  // namespace graphir

#endif  // GRAPHIR_SUPPORT_AFFINECONTAINER_H