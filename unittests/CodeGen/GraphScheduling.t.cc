#include "graphir/CodeGen/BGL.h"
#include "boost/concept/assert.hpp"
#include "boost/graph/graph_concepts.hpp"
#include "graphir/Graph/NodeUtils.h"
#include "graphir/CodeGen/GraphScheduling.h"
#include "gtest/gtest.h"
#include <fstream>

using namespace graphir;

TEST(CodeGenUnitTest, TestGraphScheduleConcepts) {
  // requirement for DFS
  BOOST_CONCEPT_ASSERT(( boost::VertexListGraphConcept<graphir::GraphSchedule> ));
  BOOST_CONCEPT_ASSERT(( boost::IncidenceGraphConcept<graphir::GraphSchedule> ));
  // additional requirement for Graphviz
  BOOST_CONCEPT_ASSERT(( boost::EdgeListGraphConcept<graphir::GraphSchedule> ));
  BOOST_CONCEPT_ASSERT(( boost::ReadablePropertyMapConcept<
                          graphir::graph_id_map<graphir::GraphSchedule,
                                              boost::vertex_index_t>,
                          graphir::BasicBlock* > ));
}

TEST(CodeGenUnitTest, GraphScheduleCFGSimpleCtrl) {
  Graph G;
  auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
               .FuncName("func_schedule_cfg_simple_ctrl")
               .Build();
  auto* Const = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
  auto* Branch = NodeBuilder<IrOpcode::If>(&G)
                 .Condition(Const).Build();
  Branch->appendControlInput(Func);
  auto* TrueBr = NodeBuilder<IrOpcode::VirtIfBranches>(&G, true)
                 .IfStmt(Branch).Build();
  auto* FalseBr = NodeBuilder<IrOpcode::VirtIfBranches>(&G, false)
                  .IfStmt(Branch).Build();
  auto* Branch2 = NodeBuilder<IrOpcode::If>(&G)
                  .Condition(Const).Build();
  Branch2->appendControlInput(TrueBr);
  auto* TrueBr2 = NodeBuilder<IrOpcode::VirtIfBranches>(&G, true)
                  .IfStmt(Branch2).Build();
  auto* FalseBr2 = NodeBuilder<IrOpcode::VirtIfBranches>(&G, false)
                   .IfStmt(Branch2).Build();
  auto* MergeInner = NodeBuilder<IrOpcode::Merge>(&G)
                     .AddCtrlInput(TrueBr2).AddCtrlInput(FalseBr2)
                     .Build();
  auto* MergeOuter = NodeBuilder<IrOpcode::Merge>(&G)
                     .AddCtrlInput(FalseBr).AddCtrlInput(MergeInner)
                     .Build();
  auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
              .AddTerminator(MergeOuter)
              .Build();
  SubGraph FuncSG(End);
  G.AddSubRegion(FuncSG);
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleCtrl.dot");
    G.dumpGraphviz(OF);
  }

  GraphScheduler Scheduler(G);
  Scheduler.ComputeScheduledGraph();
  EXPECT_EQ(Scheduler.schedule_size(), 1);
  auto* FuncSchedule = *Scheduler.schedule_begin();
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleCtrl.after.dot");
    FuncSchedule->dumpGraphviz(OF);
  }
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleCtrl.after.dom.dot");
    FuncSchedule->dumpDomTreeGraphviz(OF);
  }
}

TEST(CodeGenUnitTest, GraphScheduleCFGSimpleLoop) {
  Graph G;
  auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
               .FuncName("func_schedule_cfg_simple_loop")
               .Build();
  auto* Const = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
  auto* Loop = NodeBuilder<IrOpcode::Loop>(&G, Func)
               .Condition(Const).Build();
  auto* Br = NodeProperties<IrOpcode::Loop>(Loop).Branch();
  auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
              .AddTerminator(NodeProperties<IrOpcode::If>(Br).FalseBranch())
              .Build();
  SubGraph FuncSG(End);
  G.AddSubRegion(FuncSG);
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleLoop.dot");
    G.dumpGraphviz(OF);
  }

  GraphScheduler Scheduler(G);
  Scheduler.ComputeScheduledGraph();
  EXPECT_EQ(Scheduler.schedule_size(), 1);
  auto* FuncSchedule = *Scheduler.schedule_begin();
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleLoop.after.dot");
    FuncSchedule->dumpGraphviz(OF);
  }
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleLoop.after.dom.dot");
    FuncSchedule->dumpDomTreeGraphviz(OF);
  }
  {
    std::ofstream OF("TestGraphScheduleCFGSimpleLoop.after.looptree.dot");
    FuncSchedule->dumpLoopTreeGraphviz(OF);
  }
}

TEST(CodeGenUnitTest, GraphScheduleCFGNestedLoop) {
  Graph G;
  auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
               .FuncName("func_schedule_cfg_nested_loop")
               .Build();
  auto* Const = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
  auto* Loop1 = NodeBuilder<IrOpcode::Loop>(&G, Func)
                .Condition(Const).Build();
  auto* Br1 = NodeProperties<IrOpcode::Loop>(Loop1).Branch();
  auto* False1 = NodeProperties<IrOpcode::If>(Br1).FalseBranch();
  auto* True1 = NodeProperties<IrOpcode::If>(Br1).TrueBranch();

  auto* Loop1_1 = NodeBuilder<IrOpcode::Loop>(&G, True1)
                  .Condition(Const).Build();
  auto* Br1_1 = NodeProperties<IrOpcode::Loop>(Loop1_1).Branch();
  auto* False1_1 = NodeProperties<IrOpcode::If>(Br1_1).FalseBranch();

  auto* Loop1_2 = NodeBuilder<IrOpcode::Loop>(&G, False1_1)
                  .Condition(Const).Build();
  auto* Br1_2 = NodeProperties<IrOpcode::Loop>(Loop1_2).Branch();
  auto* False1_2 = NodeProperties<IrOpcode::If>(Br1_2).FalseBranch();
  Loop1->ReplaceUseOfWith(True1, False1_2, Use::K_CONTROL);

  auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
              .AddTerminator(False1)
              .Build();
  SubGraph FuncSG(End);
  G.AddSubRegion(FuncSG);
  {
    std::ofstream OF("TestGraphScheduleCFGNestedLoop.dot");
    G.dumpGraphviz(OF);
  }

  GraphScheduler Scheduler(G);
  Scheduler.ComputeScheduledGraph();
  EXPECT_EQ(Scheduler.schedule_size(), 1);
  auto* FuncSchedule = *Scheduler.schedule_begin();
  {
    std::ofstream OF("TestGraphScheduleCFGNestedLoop.after.dot");
    FuncSchedule->dumpGraphviz(OF);
  }
  {
    std::ofstream OF("TestGraphScheduleCFGNestedLoop.after.dom.dot");
    FuncSchedule->dumpDomTreeGraphviz(OF);
  }
  {
    std::ofstream OF("TestGraphScheduleCFGNestedLoop.after.looptree.dot");
    FuncSchedule->dumpLoopTreeGraphviz(OF);
  }
}

TEST(CodeGenUnitTest, GraphScheduleValueNodePlacement) {
  {
    Graph G;
    auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
                 .FuncName("func_schedule_value_node_placement")
                 .Build();
    auto* Const1 = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
    auto* Const2 = NodeBuilder<IrOpcode::ConstantInt>(&G, 2).Build();
    auto* Const3 = NodeBuilder<IrOpcode::ConstantInt>(&G, 3).Build();
    auto* Const4 = NodeBuilder<IrOpcode::ConstantInt>(&G, 4).Build();
    auto* Const5 = NodeBuilder<IrOpcode::ConstantInt>(&G, 5).Build();
    auto* Sum1 = NodeBuilder<IrOpcode::BinAdd>(&G)
                 .LHS(Const1).RHS(Const2).Build();
    auto* Mul1 = NodeBuilder<IrOpcode::BinMul>(&G)
                 .LHS(Sum1).RHS(Const3).Build();

    auto* Branch = NodeBuilder<IrOpcode::If>(&G)
                   .Condition(Const1).Build();
    Branch->appendControlInput(Func);
    auto* TrueBr = NodeBuilder<IrOpcode::VirtIfBranches>(&G, true)
                   .IfStmt(Branch).Build();
    auto* FalseBr = NodeBuilder<IrOpcode::VirtIfBranches>(&G, false)
                    .IfStmt(Branch).Build();
    auto* Return1 = NodeBuilder<IrOpcode::Return>(&G, Sum1).Build();
    Return1->appendControlInput(FalseBr);

    auto* Branch2 = NodeBuilder<IrOpcode::If>(&G)
                    .Condition(Const1).Build();
    Branch2->appendControlInput(TrueBr);
    auto* TrueBr2 = NodeBuilder<IrOpcode::VirtIfBranches>(&G, true)
                    .IfStmt(Branch2).Build();
    auto* FalseBr2 = NodeBuilder<IrOpcode::VirtIfBranches>(&G, false)
                     .IfStmt(Branch2).Build();
    auto* Sum2_1 = NodeBuilder<IrOpcode::BinAdd>(&G)
                   .LHS(Mul1).RHS(Const4).Build();
    auto* Sum2_2 = NodeBuilder<IrOpcode::BinAdd>(&G)
                   .LHS(Mul1).RHS(Const5).Build();
    auto* MergeInner = NodeBuilder<IrOpcode::Merge>(&G)
                       .AddCtrlInput(TrueBr2).AddCtrlInput(FalseBr2)
                       .Build();
    auto* PHINode = NodeBuilder<IrOpcode::Phi>(&G)
                    .AddValueInput(Sum2_1).AddValueInput(Sum2_2)
                    .SetCtrlMerge(MergeInner)
                    .Build();

    auto* MergeOuter = NodeBuilder<IrOpcode::Merge>(&G)
                       .AddCtrlInput(Return1).AddCtrlInput(MergeInner)
                       .Build();
    auto* Return2 = NodeBuilder<IrOpcode::Return>(&G, PHINode).Build();
    Return2->appendControlInput(MergeOuter);
    auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
                .AddTerminator(Return2)
                .Build();
    SubGraph FuncSG(End);
    G.AddSubRegion(FuncSG);
    {
      std::ofstream OF("TestGraphScheduleValueNodePlacement1.dot");
      G.dumpGraphviz(OF);
    }

    GraphScheduler Scheduler(G);
    Scheduler.ComputeScheduledGraph();
    EXPECT_EQ(Scheduler.schedule_size(), 1);
    auto* FuncSchedule = *Scheduler.schedule_begin();
    {
      std::ofstream OF("TestGraphScheduleValueNodePlacement1.after.dot");
      FuncSchedule->dumpGraphviz(OF);
    }
    {
      std::ofstream OF("TestGraphScheduleValueNodePlacement1.after.dom.dot");
      FuncSchedule->dumpDomTreeGraphviz(OF);
    }
  }
  {
    Graph G;
    auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
                 .FuncName("func_schedule_value_node_placement2")
                 .Build();
    auto* Const1 = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
    auto* Const2 = NodeBuilder<IrOpcode::ConstantInt>(&G, 2).Build();
    auto* Const3 = NodeBuilder<IrOpcode::ConstantInt>(&G, 3).Build();
    auto* Const4 = NodeBuilder<IrOpcode::ConstantInt>(&G, 4).Build();
    auto* Sum1 = NodeBuilder<IrOpcode::BinMul>(&G)
                 .LHS(Const3).RHS(Const4).Build();
    auto* Sum2 = NodeBuilder<IrOpcode::BinAdd>(&G)
                 .LHS(Sum1).RHS(Const2).Build();

    auto* Loop = NodeBuilder<IrOpcode::Loop>(&G, Func)
                 .Condition(Const1).Build();
    auto* PHINode = NodeBuilder<IrOpcode::Phi>(&G)
                    .AddValueInput(Const2).AddValueInput(Sum2)
                    .SetCtrlMerge(Loop)
                    .Build();
    Sum2->ReplaceUseOfWith(Const2, PHINode, Use::K_VALUE);
    auto* Br = NodeProperties<IrOpcode::Loop>(Loop).Branch();
    auto* Return1 = NodeBuilder<IrOpcode::Return>(&G, PHINode).Build();
    Return1->appendControlInput(NodeProperties<IrOpcode::If>(Br).FalseBranch());
    auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
                .AddTerminator(Return1)
                .Build();

    SubGraph FuncSG(End);
    G.AddSubRegion(FuncSG);
    {
      std::ofstream OF("TestGraphScheduleValueNodePlacement2.dot");
      G.dumpGraphviz(OF);
    }

    GraphScheduler Scheduler(G);
    Scheduler.ComputeScheduledGraph();
    EXPECT_EQ(Scheduler.schedule_size(), 1);
    auto* FuncSchedule = *Scheduler.schedule_begin();
    {
      std::ofstream OF("TestGraphScheduleValueNodePlacement2.after.dot");
      FuncSchedule->dumpGraphviz(OF);
    }
    {
      std::ofstream OF("TestGraphScheduleValueNodePlacement2.after.dom.dot");
      FuncSchedule->dumpDomTreeGraphviz(OF);
    }
  }
}

TEST(CodeGenUnitTest, GraphScheduleMemNodePlacement) {
  {
    Graph G;
    auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
                 .FuncName("func_schedule_mem_node_placement1")
                 .Build();
    auto* Const0 = NodeBuilder<IrOpcode::ConstantInt>(&G, 0).Build();
    auto* Const1 = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
    auto* Const2 = NodeBuilder<IrOpcode::ConstantInt>(&G, 2).Build();
    auto* Const3 = NodeBuilder<IrOpcode::ConstantInt>(&G, 3).Build();
    auto* Const4 = NodeBuilder<IrOpcode::ConstantInt>(&G, 4).Build();
    auto* Const5 = NodeBuilder<IrOpcode::ConstantInt>(&G, 5).Build();

    auto* AllocaFoo = NodeBuilder<IrOpcode::Alloca>(&G)
                      .Size(Const5).Build();
    auto* AllocaBar = NodeBuilder<IrOpcode::Alloca>(&G)
                      .Size(Const5).Build();
    auto* StoreFoo1 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaFoo).Offset(Const1)
                      .Src(Const0).Build();
    auto* StoreBar1 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaBar).Offset(Const1)
                      .Src(Const0).Build();

    // Branch
    auto* Branch = NodeBuilder<IrOpcode::If>(&G)
                   .Condition(Const1).Build();
    Branch->appendControlInput(Func);

    // True
    auto* TrueBr = NodeBuilder<IrOpcode::VirtIfBranches>(&G, true)
                   .IfStmt(Branch).Build();
    auto* LoadFoo1 = NodeBuilder<IrOpcode::MemLoad>(&G)
                     .BaseAddr(AllocaFoo).Offset(Const2)
                     .Build();
    LoadFoo1->appendEffectInput(StoreFoo1);
    auto* SumTrue = NodeBuilder<IrOpcode::BinAdd>(&G)
                    .LHS(LoadFoo1).RHS(Const5).Build();
    auto* StoreFoo2 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaFoo).Offset(Const3)
                      .Src(Const4).Build();
    StoreFoo2->appendEffectInput(LoadFoo1);

    // False
    auto* FalseBr = NodeBuilder<IrOpcode::VirtIfBranches>(&G, false)
                    .IfStmt(Branch).Build();
    auto* LoadBar1 = NodeBuilder<IrOpcode::MemLoad>(&G)
                     .BaseAddr(AllocaBar).Offset(Const2)
                     .Build();
    LoadBar1->appendEffectInput(StoreBar1);
    auto* SumFalse = NodeBuilder<IrOpcode::BinAdd>(&G)
                     .LHS(LoadBar1).RHS(Const5).Build();
    auto* StoreBar2 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaBar).Offset(Const2)
                      .Src(Const5).Build();
    StoreBar2->appendEffectInput(LoadBar1);

    // Merge
    auto* Merge = NodeBuilder<IrOpcode::Merge>(&G)
                  .AddCtrlInput(TrueBr).AddCtrlInput(FalseBr)
                  .Build();
    auto* ValuePHI = NodeBuilder<IrOpcode::Phi>(&G)
                     .AddValueInput(SumTrue).AddValueInput(SumFalse)
                     .SetCtrlMerge(Merge)
                     .Build();
    auto* EffectPHIFoo = NodeBuilder<IrOpcode::Phi>(&G)
                         .AddEffectInput(StoreFoo2).AddEffectInput(StoreFoo1)
                         .SetCtrlMerge(Merge)
                         .Build();
    auto* EffectPHIBar = NodeBuilder<IrOpcode::Phi>(&G)
                         .AddEffectInput(StoreBar1).AddEffectInput(StoreBar2)
                         .SetCtrlMerge(Merge)
                         .Build();

    // Follow
    // FIXME: These two will scheduled after Return node
    auto* StoreFoo3 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaFoo).Offset(Const3)
                      .Src(Const4).Build();
    StoreFoo3->appendEffectInput(EffectPHIFoo);
    auto* StoreBar3 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaBar).Offset(Const4)
                      .Src(Const5).Build();
    StoreBar3->appendEffectInput(EffectPHIBar);

    // End
    auto* Return = NodeBuilder<IrOpcode::Return>(&G, ValuePHI)
                   .Build();
    Return->appendControlInput(Merge);
    auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
                .AddTerminator(Return)
                .AddEffectDep(StoreFoo3).AddEffectDep(StoreBar3)
                .Build();
    SubGraph FuncSG(End);
    G.AddSubRegion(FuncSG);
    {
      std::ofstream OF("TestGraphScheduleMemNodePlacement1.dot");
      G.dumpGraphviz(OF);
    }

    GraphScheduler Scheduler(G);
    Scheduler.ComputeScheduledGraph();
    EXPECT_EQ(Scheduler.schedule_size(), 1);
    auto* FuncSchedule = *Scheduler.schedule_begin();
    {
      std::ofstream OF("TestGraphScheduleMemNodePlacement1.after.dot");
      FuncSchedule->dumpGraphviz(OF);
    }
    {
      std::ofstream OF("TestGraphScheduleMemNodePlacement1.after.dom.dot");
      FuncSchedule->dumpDomTreeGraphviz(OF);
    }
  }
  {
    Graph G;
    auto* Func = NodeBuilder<IrOpcode::VirtFuncPrototype>(&G)
                 .FuncName("func_schedule_mem_node_placement2")
                 .Build();
    auto* Const0 = NodeBuilder<IrOpcode::ConstantInt>(&G, 0).Build();
    auto* Const1 = NodeBuilder<IrOpcode::ConstantInt>(&G, 1).Build();
    auto* Const2 = NodeBuilder<IrOpcode::ConstantInt>(&G, 2).Build();
    auto* Const3 = NodeBuilder<IrOpcode::ConstantInt>(&G, 3).Build();
    auto* Const4 = NodeBuilder<IrOpcode::ConstantInt>(&G, 4).Build();
    auto* Const5 = NodeBuilder<IrOpcode::ConstantInt>(&G, 5).Build();

    auto* AllocaFoo = NodeBuilder<IrOpcode::Alloca>(&G)
                      .Size(Const5).Build();
    auto* AllocaBar = NodeBuilder<IrOpcode::Alloca>(&G)
                      .Size(Const5).Build();
    auto* StoreFoo1 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaFoo).Offset(Const1)
                      .Src(Const0).Build();
    auto* StoreBar1 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaBar).Offset(Const1)
                      .Src(Const0).Build();

    // Loop
    auto* Loop = NodeBuilder<IrOpcode::Loop>(&G, Func)
                 .Condition(Const1).Build();
    auto* StoreFoo2 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaFoo).Offset(Const2)
                      .Src(Const1).Build();
    StoreFoo2->appendEffectInput(StoreFoo1);
    auto* LoadFoo2 = NodeBuilder<IrOpcode::MemLoad>(&G)
                     .BaseAddr(AllocaFoo).Offset(Const2)
                     .Build();
    LoadFoo2->appendEffectInput(StoreFoo2);
    auto* StoreBar2 = NodeBuilder<IrOpcode::MemStore>(&G)
                      .BaseAddr(AllocaBar).Offset(Const2)
                      .Src(LoadFoo2).Build();
    StoreBar2->appendEffectInput(StoreBar1);
    auto* PHIFoo = NodeBuilder<IrOpcode::Phi>(&G)
                   .AddEffectInput(StoreFoo1).AddEffectInput(LoadFoo2)
                   .SetCtrlMerge(Loop).Build();
    auto* PHIBar = NodeBuilder<IrOpcode::Phi>(&G)
                   .AddEffectInput(StoreBar1).AddEffectInput(StoreBar2)
                   .SetCtrlMerge(Loop).Build();
    StoreFoo2->ReplaceUseOfWith(StoreFoo1, PHIFoo, Use::K_EFFECT);
    StoreBar2->ReplaceUseOfWith(StoreBar1, PHIBar, Use::K_EFFECT);

    // End
    auto* LoadFoo3 = NodeBuilder<IrOpcode::MemLoad>(&G)
                     .BaseAddr(AllocaFoo).Offset(Const3)
                     .Build();
    LoadFoo3->appendEffectInput(PHIFoo);
    auto* LoadBar2 = NodeBuilder<IrOpcode::MemLoad>(&G)
                     .BaseAddr(AllocaBar).Offset(Const4)
                     .Build();
    LoadBar2->appendEffectInput(PHIBar);
    auto* Sum = NodeBuilder<IrOpcode::BinAdd>(&G)
                .LHS(LoadFoo3).RHS(LoadBar2)
                .Build();
    auto* Return = NodeBuilder<IrOpcode::Return>(&G, Sum)
                   .Build();

    NodeProperties<IrOpcode::Loop> LNP(Loop);
    NodeProperties<IrOpcode::If> BNP(LNP.Branch());
    Return->appendControlInput(BNP.FalseBranch());
    auto* End = NodeBuilder<IrOpcode::End>(&G, Func)
                .AddTerminator(Return)
                .Build();
    SubGraph FuncSG(End);
    G.AddSubRegion(FuncSG);
    {
      std::ofstream OF("TestGraphScheduleMemNodePlacement2.dot");
      G.dumpGraphviz(OF);
    }

    GraphScheduler Scheduler(G);
    Scheduler.ComputeScheduledGraph();
    EXPECT_EQ(Scheduler.schedule_size(), 1);
    auto* FuncSchedule = *Scheduler.schedule_begin();
    {
      std::ofstream OF("TestGraphScheduleMemNodePlacement2.after.dot");
      FuncSchedule->dumpGraphviz(OF);
    }
    {
      std::ofstream OF("TestGraphScheduleMemNodePlacement2.after.dom.dot");
      FuncSchedule->dumpDomTreeGraphviz(OF);
    }
  }
}
