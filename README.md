# graph-ir

PL241

Graph用来存储所有的封闭Node，同时也是它们的所有者。
SubGraph表示函数内节点的子集，只存储该函数的End节点，很容易进行构造和复制。

边（Edge）
---
一条边代表一种依赖关系。依赖于Source节点的sink将形成从sink到Source的一个直接边缘点。
graphir中有三类边:

- **Value**: 表示正常的数据流。接收节点将使用源节点生成的数据。
- **Control**: 表示执行模型的virtual control token的移动路径。
- **Effect**: 表示强制定义两个节点之间的顺序。如果MemLoad节点需要在MemStore之后执行，则MemLoad节点将依赖于MemStore。

节点（Node）
---
一个节点可以意味着一个操作或者一个外部数据(例如Argument)或者控制步骤(即所有控制节点)或者用于特殊目的的虚拟占位符(例如EffectMerge)。
节点可以有各种依赖关系例如数据输入。

优化（Optimizer）
---
 - **ValuePromotion**: llvm中`mem2reg`。
 - **Peephole**: 执行许多琐碎的图约简，如常数合并。
 - **MemoryLegalize** and **DLXMemoryLegalize**: 将内存节点合法化为以后管道中可接受的形式。
 - **CSE** 执行公共子表达式消除。

代码生成（CodeGen）
---
与传统的codegen pipeline最大的不同是不具备独立指令选择阶段。相反，我们把它分成两部分
单独的阶段:前和后机器降低。这样做的主要原因是因为我们的语言很简单，这样我们就可以在线性化图之前为大多数操作选择本地指令。