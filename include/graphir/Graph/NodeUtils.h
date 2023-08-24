#ifndef GRAPHIR_GRAPH_NODEUTILS_H
#define GRAPHIR_GRAPH_NODEUTILS_H

#include <string>

#include "graphir/Graph/Attribute.h"
#include "graphir/Graph/NodeUtilsBase.h"
#include "graphir/Support/STLExtras.h"
#include "graphir/Support/iterator_range.h"
#include "graphir/Support/type_traits.h"

namespace graphir {

// clang-format off

NODE_PROPERTIES(ConstantInt) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(ConstantInt, N) {}

  template<typename T>
  T as(const Graph& G) const {
    if(!*this) return T();
    if(auto* V = G.const_number_pool_.find_value(node_ptr_))
      return static_cast<T>(*V);
    else
      return T();
  }
};
NODE_PROPERTIES(ConstantStr) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(ConstantStr, N) {}

  const std::string& str(const Graph& G) const {
    assert(*this && "Invalid Node");
    const auto* V = G.const_str_pool_.find_value(node_ptr_);
    assert(V && "string not found");
    return *V;
  }
  const std::string str_val(const Graph& G) const {
    if(!*this) return "";
    if(const auto* V = G.const_str_pool_.find_value(node_ptr_))
      return *V;
    else
      return "";
  }
};

NODE_PROPERTIES(FunctionStub) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(FunctionStub, N) {}

  Node* getFunctionStart(const Graph& G) const {
    assert(*this && "Invalid node");
    auto* SGPtr = G.func_stub_pool_.find_value(node_ptr_);
    assert(SGPtr && "subgraph not found");
    auto& SG = *SGPtr;
    auto* EndNode = SubGraph::GetNodeFromIt(SG.node_begin());
    auto StartIt = graphir::find_if(EndNode->inputs(),
                                  [](Node* N) -> bool {
                                    return N->getOp() == IrOpcode::Start;
                                  });
    if(StartIt != EndNode->input_end())
      return *StartIt;
    else
      return nullptr;
  }

  template<Attr AT>
  bool hasAttribute(const Graph& G, Node* Func = nullptr) {
    if(!Func)
      Func = getFunctionStart(G);
    assert(Func);
    auto& Attrs = G.attributes_;
    if(!Attrs.count(Func)) return false;
    for(auto& AttrPtr : Attrs.at(Func)) {
      if(AttrPtr->Kind() == AT) return true;
    }
    return false;
  }
};

NODE_PROPERTIES(Call) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Call, N) {}

  Node* getFuncStub() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getValueInput(0);
  }

  size_t getNumParameters() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getNumValueInput() - 1;
  }

  using param_iterator = typename Node::input_iterator;
  param_iterator param_begin() {
    return std::next(node_ptr_->value_input_begin(), 1);
  }
  param_iterator param_end() {
    return node_ptr_->value_input_end();
  }
  iterator_range<param_iterator> params() {
    return make_range(param_begin(), param_end());
  }
};

NODE_PROPERTIES(VirtSrcDecl) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtSrcDecl, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    auto Op = node_ptr_->getOp();
    return Op == IrOpcode::SrcVarDecl ||
           Op == IrOpcode::SrcArrayDecl;
  }

  Node* getSymbolName() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getValueInput(0);
  }

  const std::string& ident_name(const Graph& G) const {
    auto* SymStrNode = getSymbolName();
    return NodeProperties<IrOpcode::ConstantStr>(SymStrNode)
           .str(G);
  }

};

NODE_PROPERTIES_VIRT(SrcVarDecl, VirtSrcDecl) {
  NodeProperties(Node *N)
    : NODE_PROP_VIRT(VirtSrcDecl, N) {}

  operator bool() const {
    return node_ptr_ && node_ptr_->getOp() == IrOpcode::SrcVarDecl;
  }
};

NODE_PROPERTIES_VIRT(SrcArrayDecl, VirtSrcDecl) {
  NodeProperties(Node *N)
    : NODE_PROP_VIRT(VirtSrcDecl, N) {}

  operator bool() const {
    return node_ptr_ && node_ptr_->getOp() == IrOpcode::SrcArrayDecl;
  }

  size_t dim_size() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getNumValueInput() - 1U;
  }
  Node* dim(size_t idx) const {
    assert(idx < dim_size() && "dim index out-of-bound");
    return node_ptr_->getValueInput(idx + 1);
  }
  iterator_range<typename Node::input_iterator>
  dims() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return make_range(
      node_ptr_->value_input_begin() + 1,
      node_ptr_->value_input_end()
    );
  }
};

NODE_PROPERTIES(Start) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Start, N) {}

  const std::string& name(const Graph& G) const {
    assert(node_ptr_->getNumValueInput() > 0);
    auto* NameNode = node_ptr_->getValueInput(0);
    assert(NameNode && NameNode->getOp() == IrOpcode::ConstantStr);
    return NodeProperties<IrOpcode::ConstantStr>(NameNode)
           .str(G);
  }

  Node* EndNode() const {
    for(auto* CU : node_ptr_->users())
      if(CU->getOp() == IrOpcode::End) return CU;
    return nullptr;
  }

  Node* FuncStub(Graph& G) const {
    auto* End = EndNode();
    assert(End);
    return G.func_stub_pool_.find_node(SubGraph(End));
  }
};

NODE_PROPERTIES(Argument) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Argument, N) {}
  Node* getFuncStart() const {
    assert(node_ptr_->effect_users().begin()
           != node_ptr_->effect_users().end());
    auto* Start = *node_ptr_->effect_users().begin();
    assert(Start->getOp() == IrOpcode::Start);
    return Start;
  }
};

NODE_PROPERTIES(VirtSrcDesigAccess) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtSrcDesigAccess, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    auto Op = node_ptr_->getOp();
    return Op == IrOpcode::SrcVarAccess ||
           Op == IrOpcode::SrcArrayAccess;
  }

  Node* decl() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getValueInput(0);
  }

  Node* effect_dependency() const {
    if(node_ptr_->getNumEffectInput() > 0)
      return node_ptr_->getEffectInput(0);
    else
      return nullptr;
  }
};

template<>
struct NodeProperties<IrOpcode::SrcVarAccess>
  : public NodeProperties<IrOpcode::VirtSrcDesigAccess> {
  NodeProperties(Node *N)
    : NodeProperties<IrOpcode::VirtSrcDesigAccess>(N) {}

  operator bool() const {
    return node_ptr_ && node_ptr_->op_ == IrOpcode::SrcVarAccess;
  }
};

template<>
struct NodeProperties<IrOpcode::SrcArrayAccess>
  : public NodeProperties<IrOpcode::VirtSrcDesigAccess> {
  NodeProperties(Node *N)
    : NodeProperties<IrOpcode::VirtSrcDesigAccess>(N) {}

  operator bool() const {
    return node_ptr_ && node_ptr_->op_ == IrOpcode::SrcArrayAccess;
  }

  size_t dim_size() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getNumValueInput() - 1U;
  }
  Node* dim(size_t idx) const {
    assert(idx < dim_size() && "dim index out-of-bound");
    return node_ptr_->getValueInput(idx + 1);
  }
  iterator_range<typename Node::input_iterator>
  dims() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return make_range(
      node_ptr_->value_input_begin() + 1,
      node_ptr_->value_input_end()
    );
  }
};

NODE_PROPERTIES(SrcAssignStmt) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(SrcAssignStmt, N) {}

  Node* source() const {
    assert(node_ptr_->getNumValueInput() > 1);
    return node_ptr_->getValueInput(1);
  }
  Node* dest() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getValueInput(0);
  }
};

NODE_PROPERTIES(If) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(If, N) {}

  Node* Condition() {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getValueInput(0);
  }

  Node* TrueBranch() {
    for(auto* CU : node_ptr_->control_users()) {
      if(CU->getOp() == IrOpcode::IfTrue)
        return CU;
    }
    return nullptr;
  }
  Node* FalseBranch() {
    // IfFalse or 'not true branch'
    for(auto* CU : node_ptr_->control_users()) {
      if(CU->getOp() == IrOpcode::IfFalse)
        return CU;
    }
    for(auto* CU : node_ptr_->control_users()) {
      if(CU->getOp() != IrOpcode::IfTrue)
        return CU;
    }
    return nullptr;
  }
};

NODE_PROPERTIES(Merge) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Merge, N) {}

  // fortunetly our language is simple enough
  // to have only two branches at all time
  Node* TrueBranch() const {
    assert(node_ptr_->getNumControlInput() > 0);
    for(unsigned i = 0, CSize = node_ptr_->getNumControlInput();
        i < CSize; ++i) {
      Node* N = node_ptr_->getControlInput(i);
      if(NodeProperties<IrOpcode::IfTrue>(N)) return N;
    }
    return nullptr;
  }
  // return the If node if there is no IfFalse node and Fallthrough is true
  Node* FalseBranch(bool Fallthrough = false) const {
    for(unsigned i = 0, CSize = node_ptr_->getNumControlInput();
        i < CSize; ++i) {
      Node* N = node_ptr_->getControlInput(i);
      if(NodeProperties<IrOpcode::IfFalse>(N)) return N;
      else if(NodeProperties<IrOpcode::If>(N) && Fallthrough) return N;
    }
    return nullptr;
  }
};

NODE_PROPERTIES(VirtIfBranches) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtIfBranches, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    auto Op = node_ptr_->getOp();
    return Op == IrOpcode::IfTrue ||
           Op == IrOpcode::IfFalse;
  }

  Node* BranchPoint() const {
    for(auto* N : node_ptr_->control_inputs()) {
      if(NodeProperties<IrOpcode::If>(N))
        return N;
    }
    return nullptr;
  }
};

NODE_PROPERTIES(Loop) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Loop, N) {}

  Node* Branch() {
    for(auto* CU : node_ptr_->control_users()) {
      if(CU->getOp() == IrOpcode::If) return CU;
    }
    return nullptr;
  }

  Node* Backedge() {
    assert(node_ptr_->getNumControlInput() == 2);
    return node_ptr_->getControlInput(1);
  }
};

NODE_PROPERTIES(Phi) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Phi, N) {}

  Node* CtrlPivot() const {
    assert(node_ptr_->getNumControlInput() > 0);
    return node_ptr_->getControlInput(0);
  }

  using ctrl_input_iterator = typename Node::input_iterator;
  ctrl_input_iterator ctrl_begin() {
    return CtrlPivot()->control_input_begin();
  }
  ctrl_input_iterator ctrl_end() {
    return CtrlPivot()->control_input_end();
  }

  // value/effect input to Phi -> corresponding control node
  Node* MapCtrlNode(Node* N, Use::Kind InputKind = Use::K_VALUE) {
    size_t Idx = 0U;
    switch(InputKind) {
    case Use::K_VALUE: {
      auto Size = node_ptr_->getNumValueInput();
      for(; Idx < Size; ++Idx) {
        if(N == node_ptr_->getValueInput(Idx)) break;
      }
      if(Idx >= Size) return nullptr;
      break;
    }
    case Use::K_EFFECT: {
      auto Size = node_ptr_->getNumEffectInput();
      for(; Idx < Size; ++Idx) {
        if(N == node_ptr_->getEffectInput(Idx)) break;
      }
      if(Idx >= Size) return nullptr;
      break;
    }
    default:
      graphir_unreachable("Invalid input kind");
    }
    assert(Idx < CtrlPivot()->getNumControlInput());
    return CtrlPivot()->getControlInput(Idx);
  }
};

NODE_PROPERTIES(Alloca) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Alloca, N) {}

  Node* Size() const {
    assert(node_ptr_->getNumValueInput() > 0);
    return node_ptr_->getValueInput(0);
  }
};

NODE_PROPERTIES(VirtGlobalValues) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtGlobalValues, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    switch(node_ptr_->getOp()) {
    case IrOpcode::ConstantStr:
    case IrOpcode::ConstantInt:
    case IrOpcode::Start:
    case IrOpcode::End:
    case IrOpcode::Dead:
    case IrOpcode::FunctionStub:
      return true;
    default:
      return false;
    }
  }
};

NODE_PROPERTIES(VirtConstantValues) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtConstantValues, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    switch(node_ptr_->getOp()) {
    case IrOpcode::ConstantStr:
    case IrOpcode::ConstantInt:
    case IrOpcode::Dead:
      return true;
    default:
      return false;
    }
  }
};

NODE_PROPERTIES(VirtCtrlPoints) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtCtrlPoints, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    switch(node_ptr_->getOp()) {
    case IrOpcode::If:
    case IrOpcode::IfTrue:
    case IrOpcode::IfFalse:
    case IrOpcode::Merge:
    case IrOpcode::Start:
    case IrOpcode::End:
    case IrOpcode::Return:
    case IrOpcode::Loop:
      return true;
    default:
      return false;
    }
  }
};

NODE_PROPERTIES(VirtMemOps) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtMemOps, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    auto OC = node_ptr_->getOp();
    return OC == IrOpcode::MemLoad ||
           OC == IrOpcode::MemStore;
  }

  Node* BaseAddr() {
    if(node_ptr_->getNumValueInput() > 0) {
      return node_ptr_->getValueInput(0);
    }
    return nullptr;
  }
  Node* Offset() {
    if(node_ptr_->getNumValueInput() > 1) {
      return node_ptr_->getValueInput(1);
    }
    return nullptr;
  }
};

NODE_PROPERTIES_VIRT(MemLoad, VirtMemOps) {
  NodeProperties(Node *N)
    : NODE_PROP_VIRT(VirtMemOps, N) {}

  operator bool() const {
    return node_ptr_ && node_ptr_->getOp() == IrOpcode::MemLoad;
  }
};
NODE_PROPERTIES_VIRT(MemStore, VirtMemOps) {
  NodeProperties(Node *N)
    : NODE_PROP_VIRT(VirtMemOps, N) {}

  operator bool() const {
    return node_ptr_ && node_ptr_->getOp() == IrOpcode::MemStore;
  }

  Node* SrcVal() {
    if(node_ptr_->getNumValueInput() > 2) {
      return node_ptr_->getValueInput(2);
    }
    return nullptr;
  }
};

NODE_PROPERTIES(VirtBinOps) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtBinOps, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    switch(node_ptr_->getOp()) {
    case IrOpcode::BinAdd:
    case IrOpcode::BinSub:
    case IrOpcode::BinMul:
    case IrOpcode::BinDiv:
    case IrOpcode::BinLe:
    case IrOpcode::BinLt:
    case IrOpcode::BinGe:
    case IrOpcode::BinGt:
    case IrOpcode::BinEq:
    case IrOpcode::BinNe:
      return true;
    default:
      return false;
    }
  }

  bool IsCommutative() const {
    if(!node_ptr_) return false;
    switch(node_ptr_->getOp()) {
    case IrOpcode::BinAdd:
    case IrOpcode::BinMul:
    case IrOpcode::BinEq:
    case IrOpcode::BinNe:
      return true;
    default:
      return false;
    }
  }

  Node* LHS() const {
    if(node_ptr_->getNumValueInput() > 0)
      return node_ptr_->getValueInput(0);
    else
      return nullptr;
  }

  Node* RHS() const {
    if(node_ptr_->getNumValueInput() > 1)
      return node_ptr_->getValueInput(1);
    else
      return nullptr;
  }
};

NODE_PROPERTIES(Return) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(Return, N) {}

  Node* ReturnVal() const {
    if(node_ptr_->getNumValueInput() > 0)
      return node_ptr_->getValueInput(0);
    else
      return nullptr;
  }
};

NODE_PROPERTIES(VirtTerminate) {
  NodeProperties(Node *N)
    : NODE_PROP_BASE(VirtTerminate, N) {}

  operator bool() const {
    if(!node_ptr_) return false;
    switch(node_ptr_->getOp()) {
    case IrOpcode::If:
    case IrOpcode::Return:
      return true;
    default:
      return false;
    }
  }
};

// clang-format on

/// ======= Builders =======
template <>
struct NodeBuilder<IrOpcode::Dead> {
  NodeBuilder(Graph* graph) : G(graph) {}

  Node* Build() {
    if (!G->dead_node_) {
      G->dead_node_ = new Node(IrOpcode::Dead, {});
      G->InsertNode(G->dead_node_);
    }
    return G->dead_node_;
  }

 private:
  Graph* G;
};

template <>
struct NodeBuilder<IrOpcode::ConstantInt> {
  NodeBuilder(Graph* graph, int32_t val) : G(graph), Val(val) {}

  Node* Build() {
    if (auto* N = G->const_number_pool_.find_node(Val))
      return N;
    else {
      // New constant Node
      Node* NewN = new Node(IrOpcode::ConstantInt);
      G->const_number_pool_.insert({NewN, Val});
      G->InsertNode(NewN);
      return NewN;
    }
  }

 public:
  Graph* G;
  int32_t Val;
};

template <>
struct NodeBuilder<IrOpcode::ConstantStr> {
  NodeBuilder(Graph* graph, const std::string& Name)
      : G(graph), SymName(Name) {}

  Node* Build() {
    if (auto* N = G->const_str_pool_.find_node(SymName))
      return N;
    else {
      // New constant Node
      Node* NewN = new Node(IrOpcode::ConstantStr);
      G->const_str_pool_.insert({NewN, SymName});
      G->InsertNode(NewN);
      return NewN;
    }
  }

 private:
  Graph* G;
  const std::string& SymName;
};

template <>
struct NodeBuilder<IrOpcode::FunctionStub> {
  NodeBuilder(Graph* graph, const SubGraph& subgraph)
      : G(graph), SG(subgraph) {}

  // NodeBuilder& AddAttribute(Node* N) {
  //  TODO
  // return *this;
  //}

  Node* Build() {
    if (auto* N = G->func_stub_pool_.find_node(SG))
      return N;
    else {
      // New function stub node
      // TODO: attribute and node update
      Node* NewN = new Node(IrOpcode::FunctionStub);
      G->func_stub_pool_.insert({NewN, SG});
      G->InsertNode(NewN);
      return NewN;
    }
  }

 private:
  Graph* G;
  SubGraph SG;
};

template <>
struct NodeBuilder<IrOpcode::Call> {
  NodeBuilder(Graph* graph, Node* Stub) : G(graph), FuncStub(Stub) {}

  NodeBuilder& AddParam(Node* N) {
    Params.push_back(N);
    return *this;
  }

  size_t arg_size() const { return Params.size(); }

  Node* Build() {
    Params.insert(Params.begin(), FuncStub);
    auto* N = new Node(IrOpcode::Call, Params);
    for (auto* P : Params)
      P->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* FuncStub;
  std::vector<Node*> Params;
};

template <>
struct NodeBuilder<IrOpcode::SrcVarDecl> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& SetSymbolName(const std::string& Name) {
    SymName = Name;
    return *this;
  }

  Node* Build() {
    Node* SymNameNode = NodeBuilder<IrOpcode::ConstantStr>(G, SymName).Build();
    // Value dependency
    Node* VarDeclNode = new Node(IrOpcode::SrcVarDecl, {SymNameNode});
    SymNameNode->users_.push_back(VarDeclNode);
    G->InsertNode(VarDeclNode);
    return VarDeclNode;
  }

 private:
  Graph* G;
  std::string SymName;
};

template <>
struct NodeBuilder<IrOpcode::SrcArrayDecl> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& SetSymbolName(const std::string& Name) {
    SymName = Name;
    return *this;
  }

  // add new dimension
  NodeBuilder& AddDim(Node* DN) {
    Dims.push_back(DN);
    return *this;
  }
  NodeBuilder& AddConstDim(uint32_t Dim) {
    assert(Dim > 0);
    NodeBuilder<IrOpcode::ConstantInt> NB(G, static_cast<int32_t>(Dim));
    return AddDim(NB.Build());
  }
  NodeBuilder& ResetDims() {
    Dims.clear();
    return *this;
  }

  Node* Build() {
    Node* SymNode = NodeBuilder<IrOpcode::ConstantStr>(G, SymName).Build();
    // value dependency
    // 0: symbol string
    // 1..N: dimension expression
    std::vector<Node*> ValDeps{SymNode};
    ValDeps.insert(ValDeps.end(), Dims.begin(), Dims.end());
    Node* ArrDeclNode = new Node(IrOpcode::SrcArrayDecl, ValDeps);
    for (auto* N : ValDeps)
      N->users_.push_back(ArrDeclNode);
    G->InsertNode(ArrDeclNode);
    return ArrDeclNode;
  }

 private:
  Graph* G;
  std::string SymName;
  std::vector<Node*> Dims;
};

template <>
struct NodeBuilder<IrOpcode::SrcInitialArray> {
  NodeBuilder(Graph* graph, Node* Decl) : G(graph), ArrayDecl(Decl) {}

  Node* Build() {
    auto* N = new Node(IrOpcode::SrcInitialArray, {ArrayDecl});
    ArrayDecl->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* ArrayDecl;
};

namespace detail {
// (Simple)Binary Ops
template <IrOpcode::ID OC, class SubT = NodeBuilder<OC>>
struct BinOpNodeBuilder {
  BinOpNodeBuilder(Graph* graph) : G(graph) {}

  SubT& LHS(Node* N) {
    LHSNode = N;
    return downstream();
  }
  SubT& RHS(Node* N) {
    RHSNode = N;
    return downstream();
  }

  Node* Build() {
    auto* BinOp = new Node(OC, {LHSNode, RHSNode});
    LHSNode->users_.push_back(BinOp);
    RHSNode->users_.push_back(BinOp);
    G->InsertNode(BinOp);
    return BinOp;
  }

 protected:
  Graph* G;
  Node *LHSNode, *RHSNode;

  SubT& downstream() { return *static_cast<SubT*>(this); }
};
}  // namespace detail

#define TRIVIAL_BIN_OP_BUILDER(OC)                         \
  template <>                                              \
  struct NodeBuilder<IrOpcode::OC>                         \
      : public detail::BinOpNodeBuilder<IrOpcode::OC> {    \
    NodeBuilder(Graph* graph) : BinOpNodeBuilder(graph) {} \
  }

TRIVIAL_BIN_OP_BUILDER(BinAdd);
TRIVIAL_BIN_OP_BUILDER(BinSub);
TRIVIAL_BIN_OP_BUILDER(BinMul);
TRIVIAL_BIN_OP_BUILDER(BinDiv);
TRIVIAL_BIN_OP_BUILDER(BinLe);
TRIVIAL_BIN_OP_BUILDER(BinLt);
TRIVIAL_BIN_OP_BUILDER(BinGe);
TRIVIAL_BIN_OP_BUILDER(BinGt);
TRIVIAL_BIN_OP_BUILDER(BinEq);
TRIVIAL_BIN_OP_BUILDER(BinNe);
#undef TRIVIAL_BIN_OP_BUILDER

template <>
struct NodeBuilder<IrOpcode::SrcVarAccess> {
  NodeBuilder(Graph* graph) : G(graph), VarDecl(nullptr), EffectDep(nullptr) {}

  NodeBuilder& Decl(Node* N) {
    VarDecl = N;
    return *this;
  }

  NodeBuilder& Effect(Node* N) {
    EffectDep = N;
    return *this;
  }

  Node* Build(bool Verify = true) {
    if (Verify) {
      assert((NodeProperties<IrOpcode::SrcVarDecl>(VarDecl) ||
              NodeProperties<IrOpcode::Argument>(VarDecl)) &&
             "original decl should be SrcVarDecl");
    }

    std::vector<Node*> Effects;
    if (EffectDep)
      Effects.push_back(EffectDep);
    auto* N = new Node(IrOpcode::SrcVarAccess, {VarDecl},  // value inputs
                       {} /*control inputs*/, Effects /*effect inputs*/);
    VarDecl->users_.push_back(N);
    if (EffectDep)
      EffectDep->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node *VarDecl, *EffectDep;
};

template <>
struct NodeBuilder<IrOpcode::SrcArrayAccess> {
  NodeBuilder(Graph* graph) : G(graph), VarDecl(nullptr), EffectDep(nullptr) {}

  NodeBuilder& Decl(Node* D) {
    VarDecl = D;
    return *this;
  }
  NodeBuilder& Effect(Node* N) {
    EffectDep = N;
    return *this;
  }

  NodeBuilder& AppendAccessDim(Node* DimExpr) {
    Dims.push_back(DimExpr);
    return *this;
  }
  NodeBuilder& ResetDims() {
    Dims.clear();
    return *this;
  }

  Node* Build(bool Verify = true) {
    if (Verify) {
      // Make sure the amount of dims
      // matches that of original array decl
      NodeProperties<IrOpcode::SrcArrayDecl> NP(VarDecl);
      assert(NP && NP.dim_size() == Dims.size() && "illegal array decl");
    }

    // value dependency
    // 0: array decl
    // 1..N: dimension expression
    std::vector<Node*> ValDeps{VarDecl};
    ValDeps.insert(ValDeps.end(), Dims.begin(), Dims.end());
    std::vector<Node*> EffectDeps;
    if (EffectDep)
      EffectDeps.push_back(EffectDep);
    Node* ArrAccessNode = new Node(IrOpcode::SrcArrayAccess,
                                   ValDeps,      // value dependencies
                                   {},           // control dependencies
                                   EffectDeps);  // effect dependencies
    for (auto* N : ValDeps)
      N->users_.push_back(ArrAccessNode);
    if (EffectDep)
      EffectDep->users_.push_back(ArrAccessNode);
    G->InsertNode(ArrAccessNode);
    return ArrAccessNode;
  }

 private:
  Graph* G;
  Node *VarDecl, *EffectDep;
  std::vector<Node*> Dims;
};

template <>
struct NodeBuilder<IrOpcode::SrcAssignStmt> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& Dest(Node* N) {
    DestNode = N;
    return *this;
  }
  NodeBuilder& Src(Node* N) {
    SrcNode = N;
    return *this;
  }

  Node* Build() {
    auto* N = new Node(IrOpcode::SrcAssignStmt, {DestNode, SrcNode});
    DestNode->users_.push_back(N);
    SrcNode->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node *DestNode, *SrcNode;
};

template <>
struct NodeBuilder<IrOpcode::VirtIfBranches> {
  NodeBuilder(Graph* graph, bool IsTrueBr)
      : G(graph), IfNode(nullptr), BranchKind(IsTrueBr) {}

  NodeBuilder& IfStmt(Node* N) {
    IfNode = N;
    return *this;
  }

  Node* Build() {
    assert(IfNode && "If node cannot be null");
    auto* N = new Node(BranchKind ? IrOpcode::IfTrue : IrOpcode::IfFalse, {},
                       {IfNode});
    IfNode->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* IfNode;
  bool BranchKind;
};

template <>
struct NodeBuilder<IrOpcode::If> {
  NodeBuilder(Graph* graph) : G(graph), Predicate(nullptr) {}

  NodeBuilder& Condition(Node* N) {
    Predicate = N;
    return *this;
  }

  Node* Build() {
    assert(Predicate && "condition can not be null");
    auto* N = new Node(IrOpcode::If, {Predicate});
    Predicate->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* Predicate;
};

template <>
struct NodeBuilder<IrOpcode::Merge> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& AddCtrlInput(Node* N) {
    Ctrls.push_back(N);
    return *this;
  }

  Node* Build() {
    auto* N = new Node(IrOpcode::Merge, {}, Ctrls);
    for (auto* Ctrl : Ctrls)
      Ctrl->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  std::vector<Node*> Ctrls;
};

template <>
struct NodeBuilder<IrOpcode::EffectMerge> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& AddEffectInput(Node* N) {
    Effects.push_back(N);
    return *this;
  }

  Node* Build() {
    auto* N = new Node(IrOpcode::EffectMerge, {}, {}, Effects);
    for (auto* Effect : Effects)
      Effect->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  std::vector<Node*> Effects;
  Graph* G;
};

template <>
struct NodeBuilder<IrOpcode::Phi> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& AddValueInput(Node* N) {
    ValueDeps.push_back(N);
    return *this;
  }

  NodeBuilder& AddEffectInput(Node* N) {
    EffectDeps.push_back(N);
    return *this;
  }

  NodeBuilder& SetCtrlMerge(Node* N) {
    MergeNode = N;
    return *this;
  }

  Node* Build() {
    assert(MergeNode && "PHI require control merge point");
    auto* N = new Node(IrOpcode::Phi, ValueDeps, {MergeNode}, EffectDeps);
    MergeNode->users_.push_back(N);
    for (auto* VD : ValueDeps)
      VD->users_.push_back(N);
    for (auto* ED : EffectDeps)
      ED->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  std::vector<Node*> ValueDeps, EffectDeps;
  Node* MergeNode;
};

template <>
struct NodeBuilder<IrOpcode::Argument> {
  NodeBuilder(Graph* graph, const std::string& Name)
      : G(graph), SrcArgName(Name) {}

  Node* Build() {
    auto* NameStrNode =
        NodeBuilder<IrOpcode::ConstantStr>(G, SrcArgName).Build();
    auto* N = new Node(IrOpcode::Argument, {NameStrNode});
    NameStrNode->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  const std::string& SrcArgName;
};

template <>
struct NodeBuilder<IrOpcode::VirtFuncPrototype> {
  NodeBuilder(Graph* graph) : G(graph) {}

  NodeBuilder& FuncName(const std::string& NameStr) {
    NameStrNode = NodeBuilder<IrOpcode::ConstantStr>(G, NameStr).Build();
    return *this;
  }

  NodeBuilder& AddParameter(Node* N) {
    Parameters.push_back(N);
    return *this;
  }

  Node* Build(bool Verify = true) {
    if (Verify) {
      if (!NameStrNode) {
        log::Error() << "Require a name for the function\n";
        return nullptr;
      }
      for (auto* PN : Parameters)
        if (!NodeProperties<IrOpcode::Argument>(PN)) {
          log::Error() << "Expecting Argument kind Node\n";
          return nullptr;
        }
    }

    // Start node has effect dependency on arguments
    auto* StartNode = new Node(IrOpcode::Start, {NameStrNode}, {}, Parameters);
    NameStrNode->users_.push_back(StartNode);
    for (auto* PN : Parameters)
      PN->users_.push_back(StartNode);
    G->InsertNode(StartNode);
    return StartNode;
  }

 private:
  Graph* G;
  Node* NameStrNode;
  std::vector<Node*> Parameters;
};

template <>
struct NodeBuilder<IrOpcode::End> {
  NodeBuilder(Graph* graph, Node* Start) : G(graph), StartNode(Start) {}

  NodeBuilder& AddTerminator(Node* N) {
    TermNodes.push_back(N);
    return *this;
  }
  NodeBuilder& AddEffectDep(Node* N) {
    EffectDeps.push_back(N);
    return *this;
  }

  Node* Build() {
    // control dependent on terminator nodes,
    // or start node if the former one is absent
    std::vector<Node*> CtrlDeps;
    if (!TermNodes.empty())
      CtrlDeps = std::move(TermNodes);
    CtrlDeps.insert(CtrlDeps.begin(), StartNode);
    auto* N = new Node(IrOpcode::End, {}, CtrlDeps, EffectDeps);
    for (auto* TN : CtrlDeps)
      TN->users_.push_back(N);
    for (auto* E : EffectDeps)
      E->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* StartNode;
  std::vector<Node*> TermNodes, EffectDeps;
};

template <>
struct NodeBuilder<IrOpcode::Return> {
  explicit NodeBuilder(Graph* graph, Node* RetExpr = nullptr)
      : G(graph), ReturnExpr(RetExpr) {}

  Node* Build() {
    Node* N = nullptr;
    if (ReturnExpr) {
      N = new Node(IrOpcode::Return, {ReturnExpr});
      ReturnExpr->users_.push_back(N);
    } else {
      N = new Node(IrOpcode::Return, {});
    }
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* ReturnExpr;
};

template <>
struct NodeBuilder<IrOpcode::Loop> {
  NodeBuilder(Graph* graph, Node* CtrlPoint)
      : G(graph), LastCtrlPoint(CtrlPoint) {}

  NodeBuilder& Condition(Node* C) {
    Predicate = C;
    return *this;
  }

  Node* Build() {
    auto* IfNode = NodeBuilder<IrOpcode::If>(G).Condition(Predicate).Build();
    auto* IfTrue =
        NodeBuilder<IrOpcode::VirtIfBranches>(G, true).IfStmt(IfNode).Build();
    auto* IfFalse =
        NodeBuilder<IrOpcode::VirtIfBranches>(G, false).IfStmt(IfNode).Build();
    auto* LoopNode = new Node(IrOpcode::Loop, {},
                              // backedge is always behind LastCtrlPoint!
                              {LastCtrlPoint, IfTrue});
    IfNode->appendControlInput(LoopNode);
    LastCtrlPoint->users_.push_back(LoopNode);
    IfTrue->users_.push_back(LoopNode);
    G->InsertNode(LoopNode);
    return LoopNode;
  }

 private:
  Graph* G;
  Node* LastCtrlPoint;
  Node* Predicate;
};

template <>
struct NodeBuilder<IrOpcode::Alloca> {
  NodeBuilder(Graph* graph) : G(graph), AllocationSize(nullptr) {}

  NodeBuilder& Size(Node* N) {
    AllocationSize = N;
    return *this;
  }

  Node* Build() {
    if (!AllocationSize)
      // default to be one
      AllocationSize = NodeBuilder<IrOpcode::ConstantInt>(G, 1).Build();

    auto* N = new Node(IrOpcode::Alloca, {AllocationSize});
    AllocationSize->users_.push_back(N);
    G->InsertNode(N);
    return N;
  }

 private:
  Graph* G;
  Node* AllocationSize;
};

template <>
struct NodeBuilder<IrOpcode::MemLoad>
    : public internal::MemNodeBuilder<IrOpcode::MemLoad> {
  NodeBuilder(Graph* graph)
      : internal::MemNodeBuilder<IrOpcode::MemLoad>(graph) {}

  Node* Build() {
    auto* N = new Node(IrOpcode::MemLoad, {base_addr_node_, offset_node_});
    base_addr_node_->users_.push_back(N);
    offset_node_->users_.push_back(N);
    graph_->InsertNode(N);
    return N;
  }
};
template <>
struct NodeBuilder<IrOpcode::MemStore>
    : public internal::MemNodeBuilder<IrOpcode::MemStore> {
  NodeBuilder(Graph* graph)
      : internal::MemNodeBuilder<IrOpcode::MemStore>(graph) {}

  NodeBuilder& Src(Node* N) {
    SrcNode = N;
    return *this;
  }

  Node* Build() {
    auto* N =
        new Node(IrOpcode::MemStore, {base_addr_node_, offset_node_, SrcNode});
    base_addr_node_->users_.push_back(N);
    offset_node_->users_.push_back(N);
    SrcNode->users_.push_back(N);
    graph_->InsertNode(N);
    return N;
  }

 private:
  Node* SrcNode;
};

Node* FindNearestCtrlPoint(Node* N);
}

#endif  // GRAPHIR_GRAPH_NODEUTILS_H