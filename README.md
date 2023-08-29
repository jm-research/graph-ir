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