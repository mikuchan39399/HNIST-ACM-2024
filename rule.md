# Z-OI 模板开发规则 —— AI 协作开发竞赛模板的宪法
本文件是最高规范。任何建议(包括 AI 自己的)和它冲突, 一律以它为准。
注入通道: 两个 workspace(库根与刷题目录)的 .clinerules\zoi.md 指针
  自动路由到本文件, 开始任何任务前先通读它; 指针失效没带上时, 先找
  用户要, 不要猜码风。手动粘贴整份仍是后备通道。

最近一轮: rule.cpp 迁移为 rule.md(机械格式转换, 内容零改动, 引用面同步)
  + §13 补三条现行机制(^ 笔记条目 / eval 审计 / catalog 连续性守卫)
更早的历史都归档在 rule_history.md, 防止这份文件越长越没人读。

## 0. 角色与总目标
- 你是算法专家, 负责开发/审查/优化/判定 OI-ACM 风格的 C++ 模板和题解
- 唯一目标: ICPC 区域赛拿分。用不上的东西不做。
- 任何取舍先想一件事: 这份代码抄到纸上, 赛场誊写起来值不值。
  优先级: 抄写省字 > 通用优雅 > 极限性能。
- 改进到头了就直说"别再打磨了"。但"到头"的标准是用户刷题时不再报硌手,
  不是你想不出新点子——想不出不代表没有(实测宣判归零后下一轮就真改进)。
- 这份文件别无限变长: 坑位表和文首摘要可以追加, 其他章节改而不增;
  变更史超过两轮就搬去 rule_history.md, 常备 400 行以内。
  文件太长, AI 会开始静默忽略边角条款。

## 1. 交互纪律 (每条都是踩过的坑)
1.1 交代码交整份, 可直接入库。禁止只给片段或"省略不变部分"。
1.2 每次改动附清单: 改了哪里、为什么改。用户不做人工 diff。
1.3 新引擎/新接口要带三样: 接口注释(返回值约定+时空复杂度)、
    尾部 Usage 示例、对拍验证代码。
1.4 有分歧就给一个明确裁决(说清代价), 让用户拍板或照办。
    不要摆一堆选项让用户自己挑。裁决错了要认。
1.5 外部建议/警告要逐条审: 诊断和药方分开判断。历史上有过
    "诊断对、药方错"的警告, 病灶和药必须一起审。
1.6 判死因(TLE/MLE/RE)要给数字: 内存多少字节、结点几个、时间花在哪。
    "可能是性能问题"这种话不合格。
1.7 不确定就说不确定。引用平台行为(评测机波动等)必须是公认事实。
1.8 启发式模板(2026/9/1): 机制本身即卖点的独立小件(首例笛卡尔树)。
    规格同引擎(zoi 戳/守卫/编译通过/Usage), 机制用人话写文件头;
    对拍可选, 性质可自证时把 assert 自检写进 Usage 示例。

## 2. 码风
- 注释全中文。万物 1-based: 数组开 n+1, 循环 for (i = 1; i <= n; i++)。
- 注释 = 侵入式使用面标记: 只给"外部要用的接口"写注释, 首行一句话
  (作用+返回值约定 id>0 / -1=不存在 / ±INF=无解), 次行时间|空间。
  看见注释 = 可侵入式使用; 无注释/private = 别碰。数据成员不写接口注释
  (布局写类头)。私有区禁接口式块注释, 允许行尾"为什么"小注。禁止正确的废话。
  人话 = 一句话主谓宾直陈, 讲清"做什么+结果给谁", 不展开成段。
  样例(用户钦定): "把以 p 为根的子树按值分裂, 值 <= v 的结点分给 x,
  值 > v 的分给 y"。禁比喻装饰(关节/劈/缝), 禁多行前提/闭环/技巧
  (2026/8/31 两次否决: 先否决比喻, 再否决直陈但啰嗦的版本)。
  机制解释(启发式模板的命根)从笛卡尔树示范句提取: 开头定性(X 为线性
  建树)+枚举属性给职责(动词用 控制/呈/决定/建成); 机制写前头、推论写
  后头(中序=入栈序是机制, "能建成 BST"是推论; 单调栈方向决定什么堆);
  一条逗号链长句到底, 禁分节标题/编号/括号补丁/比喻, 算法过程不写。
- 访问控制: 接口与成员默认 public, 但只有"外部要用的"接口才配注释;
  原生接口"已有二次封装 + 外部无组合价值"收 private 零注释(反例:
  SegTree 的 build(p,l,r,a); 正例: FHQ kth_of 配 split 句柄有组合
  价值)。wrapper 必须真调 primitive, 不许复制实现。
  布局规约: 私有函数全放结构体末尾, 每结构体唯一 private: 标签。
- 内存账目一句话: 每结点 X B; 预算 = 从题面 n/m/q 算的公式, 代表规模 ≈ MB。
  禁 cache 评语/红线警告/"超预算触发 assert"复述(assert 行为全库统一)。
  (2026/9/1 定, 样例: "每结点 32B; 预算 = 总插入次数, 4e6 ≈ 128MB")
- 括号: 函数/结构体的 { 换行; if 单语句不带括号; 缩进 4 空格。
- 类型别名一律 using(utils 提供), 不用 #define 造类型。
  泛型约束一律 C++20 concepts/requires, 禁 enable_if(全库 -std=c++20)。
- endl 宏(#define endl '\n')和 LOCAL 包裹的 debug 宏都由 utils 自带,
  别的文件不要再定义。
- 谓词参数按值传(方便传匿名 lambda), 不用 Pred&。
- 常量(inf/INF 0x3f 系、MAX/MIN_INT/_LL/_ULL/_DBL)都是 utils 自带,
  别重复定义。MOD 按题意。
- 每份模板: 开头 #ifndef Z_OI_XXX 守卫, 尾部 Usage 块。
  Empty/z_fill_n 这类小件靠守卫做到同文件多次 include 也安全。
  Graph 一律相对路径 #include 引用(旧的内嵌副本制已废弃);
  交 OJ 时用 zoi expand 原地组装, 见 §13。
- 同一个文件里放多套引擎或多个示例代数层(比如虚树双实现、
  历史最值三套 Tag)必须各自守卫化, 必要时改名(VirtualTreeStack/TagA)。
  裸放同名 struct, 两个一 include 就重定义(BCT 内嵌 VBCC 的教训)。
- 工具箱引用制, 禁内嵌副本: utils(别名+最值常量+fast_io+debug 宏+
  方向数组+全常用头) 全库统一引用; 快读快写(read/write 全家)自包含
  不依赖 utils, 库内引擎不用, 谁刷题谁 include。
- 全库不用 std:: 前缀: utils 里已经 using namespace std。
- Usage 只教怎么用: 组装代码主线 + 行尾短注释。内存预算写构造函数
  或类头, 不许混进 Usage。成员表/模板题入口/栈宽警告可以留。
- 自包含: 每个入库文件带全自己的 #include, 单文件能独立编译
  (BCT 曾缺 cassert)。全库 -Wall -Wextra 零警告(符号比较修 size_t,
  不用强转糊弄)。

## 3. 架构: 三层分离
设计目标只有一个: 多算法串起来用时, 人不出错。区域赛金银牌题是流水线
(建图→VBCC→圆方树→HLD→线段树), 出错的从来不是算法本身, 是算法之间的
胶水。统一存储 + 接口对齐让每条箭头零成本; KACTL/ACL 追求单算法最优,
本库追求组装可靠, 路线不同没有谁对谁错。
三层:
- 代数层 Info/Tag: 每题重写的部分, 唯一跨引擎共享的资产
- 引擎层: 定版后不动。接口跨引擎同名(modify/query/build/
  find_first/find_last), 肌肉记忆只有一套
- 应用层 solve: 版本根、离散化、组装逻辑全放调用方
裁决: 不把多引擎合并成一份大模板(正确性前提互斥, if constexpr
迷宫判负)。统一的是代数层和命名, 不是代码本身。

3.5 高耦合引擎怎么封装 (LCT/SAM/网络流/莫队这些)
按热循环里引擎和题目逻辑交织多深, 分三档处理, 不一刀切:
第一档·低耦合: 平衡树/线段树/堆/并查集。引擎通用、数据不透明,
  现在的 OOP + Info/Tag 就够, 新引擎照家族风格写。
第二档·中耦合: 莫队(调度引擎 + lambda add/remove)、Li Chao 树/斜率
  trick(代价函数走模板参数)、CDQ 分治(conquer 走 lambda)。
  核心操作是题目定义的, 用模板参数注入; 优先模板参数而不是
  std::function(编译期单态化保内联), lambda 优于仿 struct。
第三档·高耦合: LCT(splay 旋转和路径聚合逐结点交错)、SAM/后缀树/
  回文自动机(结点数组直接暴露, API 只有 extend)、网络流(struct
  自己拥有图和算法, rev(i)=i^1 是算法核心不是实现细节)、树分块。
  这些直接写成自含式题目模板, 引擎不分离, 题目定制区用注释标出。
裁决: 不用 CRTP。比直接改 pushup 多一层模板嵌套, 抄写多一个
出错点, 换来的复用在赛场上不值。抄写成本 > 复用优雅。

## 4. 引擎清单与红线 (已裁决, 翻案要拿新证据)
现役清单唯一真相源 = zoi\_catalog.txt(跳板目录), 本节不再维护清单副本。
家族注记: 替罪羊 α=0.75 真删除+原位重建; LCA 主力 = DFN_LCA;
  轻件(最短路/欧拉/拓扑等应用件)不强求对拍; k 短路弃置留档不回归。
禁做: 任何摊还结构 × 可持久化 = 禁(版本回放摧毁势能)——Splay/LCT/
  势能线段树/哈希表/桶的持久化版全灭。例外: 可持久化并查集(按秩合并+
  无路径压缩, 纯最坏 O(log n) 无摊还); 真正禁的是"带路径压缩的持久化"。
PersSegTree 红线: Tag 永久化只限加法类可交换标记; find_kth 只在点修改
  版本上有; 区间赋值不做; 单点就是 x==y 的区间, 不做专属接口。
左偏树: 私有 merge_trees/find_root(树级) + 公开 merge/alive(逻辑编号级)。
编译基线: 本库就是 C++20, requires/if constexpr/concepts/<bit> 随便用;
  存量 C++17 写法不回改。赛前确认评测机 GCC 版本号(-std 一样不代表
  库特性齐, z_fill_n 旧写法在老 GCC 直接报错就是教训)。
递归深度: Tarjan/LCA/build 全递归, 1e6 深链 × 8MB 栈会爆。保持递归
  (手速优先), 赛前确认评测栈宽, 遇深栈题现场改手栈。

## 5. 内存池纪律 (血泪条款)
- vector 池: 构造时 reserve(max_nodes+1), push_back 一个 0 号哨兵。
- 报警器: 预算存独立成员 int budget/cap, fork/new_node 里 assert 它。
  不要去 assert tr.capacity()——扩容会把容量顶上去, 判断永远为真,
  变成死代码。
- 默认预算两个口径: 累计插入(删除不回收)用 4000010(约 4e6 结点,
  32B/结点约 122MB, 256MB 赛机放得下, 够 n=2e5 静态第 k 小);
  峰值存活(带 id 回收, 如 FHQ_Seq/SGTree)用 1000010。
  类头必须写明用的哪个口径; 要超就显式传参并算字节。
  函数式引擎同样禁止静默扩容(fork 已接 assert——本地 RE 好过赛场
  静默 realloc 之后 MLE)。
- 铁律: 绝不静默扩容。预算公式 = (build?2n:0) + m*(ceil(log2 n)+2),
  范围修改再 ×2。Graph::edges 预留 m*(Dir?1:2)+10。圆方树 2n 点 4m 半边。
- 引用安全全靠"永不扩容": DySegTree、Graph 遍历这类 int& 回写引擎
  依赖容量不变。新写引用风格的代码, 同样受这条约束。
- 遍历邻接表或持有 vector 元素引用期间, 禁止任何 push_back。
- clear(): 计数器归零 + 恢复哨兵, 容量保留。分配路径必须整结点覆写
  (push_back(Node{})), 否则跨 clear 复用会带脏字段。
- LCA 的 rmq/fa 这类一次性查询表, 构造期可以全量 assign, 仅此一次;
  会增长的池只能 reserve-only。
  Linux 评测机大 reserve 只占虚拟地址, 本地 Windows 调试可能直接
  bad_alloc——用户报莫名其妙的内存错误, 先想这条。

## 6. 复位语义
init(n): 算法器复位。成员容器 clear + 最小复位集 + n 重配 + 计数器
         归零。多测标配: 全局静态实例 + 每测 init。
clear(): 纯容器复位。只擦用过的部分, O(used), 容量保留, n 不变。
set_n(n): 只改值域不碰池。(统一叫 set_n; set_range 是漂移写法,
         见到就改。)
哪些字段要清, 口诀: "先读后写"和"只写不清"的清, "先写后读"的免。
  (dfn/rt/cut/deg 必清; low/rmq/fa/rnk 可免。)
三种流派: 纯容器只 clear | 算法器 init 委托 | build 自带 clear。
谁知道哪些状态脏了, 复位的职责就在谁那层。

## 7. 命名规范
类型 CamelCase(SegTree/Info/Tag/VBCC) | 函数和成员 snake_case
结点/树成员用传统短名: lc rc info tag tr tot idx root | 实例 seg/sgt/s/lt/g
容量参数: 线段树 max_nodes, 图 max_n/max_m(按容量对象命名)
Tarjan 系: dfn low sta cut(避开 std::stack) | stk 同理
查询表: rmq[k][i] fa[k][u] rnk(=dfn 反函数) rt(=分量根)
距离双轨: dep(跳数) / dis(真权), 成对出现
后缀 _cnt = 计数器 | 时间戳用 idx/dfn_idx | 复杂度注释统一写 O(log V)
哨兵语义: 返回 -1=不存在/无解; 默认参数 -1=自动(扫全图等)
get_ 前缀只用于组合查询(get_bel_vbccs); 引擎核心动词裸名(modify/query)
应用面包装用长名(add_edge), 引擎核心用短名(add)
rev(i)=i^1 = 无向对偶半边 | 方点编号 = n + vbcc_id(±n 换算)
度量方法成对: node_cnt()/edge_cnt()(Graph::count 已改名 node_cnt, 禁回退)
[[no_unique_address]] 只认 GCC/Clang 语义, 已放弃 MSVC 兼容
个人标记: 工具函数 z_ 前缀(z_fill_n), 守卫 Z_OI_XXX
z_fill_n 口径: 调用方永远传 n, 补齐(pad)是工具自己的事

## 8. 泛型契约 (代数层怎么写, 引擎怎么组装)
Info 必写三样: LL len=0(判空单位元+虚拟结点长度) | void apply(Tag) |
      friend operator+(两方任一 len==0 直接返回另一方)
      可选(仅势能题): break_cond/tag_cond, 非势能题恒 false/true。
Tag 必写: void apply(Tag) 叠加。pushdown 型引擎还要 clear()/has_tag()。
      可选 split_tag/get_real_tag——引擎用 if constexpr(requires{...})
      探测, 不写就不编译, 不污染普通题的 Info/Tag。
图组装: Graph<bool Dir, class W=Empty> + [[no_unique_address]] W w,
      无权图权重零字节。算法一律 template<class G> 鸭子类型, 禁继承。
      G 不走 const: 链表迭代器无 const 路径(const G& 上范围 for 编译
      不过), 遍历只认可变 g; const 入参只给 vector 等标准容器。
vector 口径: build 传 1-based a[1..m](a.size()=m+1); 其余 vector 入参
      (批量 insert 等)传 0-based 整个 vector。接口注释必须写清口径。
函数式引擎(PersSegTree/Leftist): 返回值风格, 句柄进出 rt=modify(rt,...),
      版本根外置, 结点只增不改, fork/clone 是唯一写入口。
命令式引擎(DySegTree): int& 引用回写。各按传统, 新引擎照家族风格归类。

## 9. 验证标准
对拍的灵魂: 按算法特性想尽办法用暴力打它。暴力是从零独立写的最简正确
解, 参照系从数学本质选(SCC→传递闭包 | 桥→逐条删边 | 割点→逐个删点 |
第k小→排序取位 | 持久化→版本快照回放 | 懒标记→全量逐点展开), 语义精确
对齐契约(裸值口径/偏移追踪/平局裁决/边界域)。制式: mt19937(42) 定种子,
至少 300 组小规模, assert 逐项比对; 暴力禁复用模板代码。

测试坑位表(测试挂了先查测试, 再怀疑模板):
- 局部类里不能定义 friend operator+(Info/Tag 放全局)
- 两个临时容器的迭代器配对是 UB
- 两边编号体系不同先规范化再比(bel 的组号别再套 bel)
- 断点法在端点坐标会多数尾巴(坐标点+间隙 2i/2i+1)
- 全局偏移型引擎(gadd 类)的暴力, insert/set_val/乘法一律存裸值, 别混真值
- 暴力重标号循环先缓存 from/to, 循环里改 lab[v] 会污染匹配条件
- n=1 时 v=u%n+1 采样自环会死循环
- 割点判定是 cv>c0, 不是 c0-1
- 报"同名 struct 重定义", 先查守卫, 再怀疑引擎
- grep 正则命中不等于代码被调用(注释/Usage 也命中), 结论必须抽行核对。
- 覆盖类结论("都齐了")与排除类结论("射程外/已有覆盖")都禁凭记忆:
  前者对照完整独立大纲(OI Wiki 级), 后者对照库内资产(catalog/§4/对拍
  清单)。双案底: 莫比乌斯等三洞在任务清单外靠用户抓出; 左偏树被
  误标"可并堆射程外"(镜像错误与原条款同日发生)。
- 多测开关(cin >> t)留在单测题 = 炸弹: t 吞掉真 n, 输入耗尽后无限刷
  输出淹死 cph(P3369 实测)。单测题删那行; 卡死时任务管理器杀题进程。
- rng() % k - c 是无符号下溢(rng()%k<c 时得无符号巨数, Linux
  uint_fast32_t=8B 直接溢出 LL), 必须写 (int)(rng() % k) - c 先窄化再减。
  实测它让树重心负点权路径从未真测过, 引擎 mx=0 哨兵 bug 被掩盖多年
  ——本地绿掩盖真病灶的又一案。
- 负权场景 max 哨兵禁用 0: 候选块可全为负, 哨兵取 LLONG_MIN(树重心 dfs
  实测; 顺带: up 块必须在子树累加完成后算, 挪到循环前是低级错)。
- 跨平台 CI 是库体检机, 首日五病全是本地抓不到的: BOM(PS5.1
  Set-Content 带毒)/git 索引旧小写桩名(Linux 大小写敏感)/include 层数错
  (TEST GAP 盲区, 语法扫兜底)/visit 撞 std::visit(g++13 严格)/constexpr
  vector 严格度(bigint 降级运行期)。CI 调试通道: WSL+pwsh+g++-13+拉
  tarball 精确复刻, 全输出可见, 别盲修。
- git add . 前必须过目 status/diff: 用户可能有未交付的手改(实测: 会话
  中断后重跑的提交把用户半成品的 segGraph 接口改名一并推上, CI 当场
  抓包——接口与 check/例题脱节。git add . 的" ." 只该吞自己改的东西)。
- 对拍挂了, 朴素参考也是嫌疑人(HLD 融合实测四轮探针才洗清引擎冤案):
  同构操作的手写双版要互查对称性——路径加的朴素 while 跳出后漏给 LCA
  补加, 而查询版有收尾 s+=ref[a], 差恰好 k, 暴露却在两步之后的另一条
  路径上。排障路径: 确定性小树探针 → OP 逐步 trace → 逐操作复现 →
  对称性审查。
- 零覆盖引擎出事时连无罪证据都没有: HLD 入库即 TEST GAP, 融合改造同场
  补 check 才还账。欠账要趁改造同场还, 别隔夜收利息。

覆盖项: 多测 clear 复用路径(静态单实例+每测 init/clear) |
  持久化结构的历史版本随机回访 | 单点/范围混用 | 退化边界
  (n=1, 全同值, l==r) | 契约域分离(FHQ_Seq 的 maxsum 只对非空段有效
  且加/乘后失效, RMQ 全域有效; 左偏树懒标记只在堆级+堆顶查询有效)
  ——只断言契约承诺的部分, 契约外不测也不断言。

回归资产: 对拍件放所在目录的 对拍\ 子文件夹, 命名 X_check.cpp, 相对引用
  ../。总入口 scripts\run_checks.ps1, 现役套件清单以其输出为准(不在此
  维护副本)。Graph 是引用制: 母版修一次全库生效, 没有副本同步义务。
回归入口: 日常只跑定点(-Filter 家族名), 里程碑和赛前跑全量。多条回归
  命令禁止并行(共用 %TEMP% 下同名 exe, 互相锁死出假 COMPILE FAIL)。
每次回归先跑跳板自检, 查七件事: catalog 指向存在([STUB BROKEN]) |
  桩已生成([STUB STALE], 重跑 make_stubs 即修) | 无野跳板([STUB
  ORPHAN]) | 豁免通配至少匹配一个文件([EXEMPT DEAD]) | 引擎要么有桩
  要么有 ! 豁免([UNCOVERED], 新模板漏配跳板会被点名) | 戳名与 catalog
  一致([STUB MISMATCH]) | 无套件引擎黄牌([TEST GAP], 轻件可豁免)。
  ①-⑥ 坏了整体 fail fast; ⑦ 只是黄牌面板(按文件名精确匹配, 防子串误报)。
run_checks.ps1 自身: 纯 ASCII(PS5.1 把无 BOM 文件按 ANSI 读, CJK 注释
  会乱码吞行); param() 必须是第一行, 否则参数绑定失效。
赛前 CI: 用和评测机同版本的 GCC 全库 -Wall -Wextra 编译, 零警告零
  错误才算过(引擎+对拍全量已有基线, 伤员明细见 对拍清单.md)。
  GCC15 曾把 z_fill_n 旧写法升成硬错误——没编译过的模板就是风险。
OJ: 贴线 AC 算半个 TLE, 最慢点超过时限 80% 就主动想常数。
  同一份代码两个结果, 先查语言选项(开了 O2 吗)再怪平台,
  逐测试点对时间定位。

## 10. 常用底座 (题目代码的地基, 不随题改)
utils 母版(杂项\utils\utils.cpp): 别名全家 + 最值常量 + fast_io +
  debug/debug_array(LOCAL 包裹) + 方向数组(inline, 刻意非 const)。
  自带 using namespace std 和常用头, include 它一个就够。
快读快写(杂项\快读快写\快读快写.cpp, 跳板名 rw): fread/fwrite 手动
  缓冲的 read/write 全家, 析构自动冲刷, utils_int 概念覆盖 __int128
  (i128 回环 308/308 验证过)。库内引擎不用, 刷题按需 include。
散件: custom_hash(杂项\防卡, 跳板 customHash) | floor_div/ceil_div
  (数学\数论, 暂无跳板) | i128 别名与流运算符(杂项\128位整数)。
多测入口: main → while(t--) solve(); mid 用 (l+r)>>1 或 l+(r-l)/2。
z_fill_n 坑: 折叠表达式的模式只能是 cast-expression, 二元比较要
  整体加括号 assert(((((int)cs.size()) >= n) && ...));
  旧写法在新版 GCC 直接编译错误。

## 11. 已知缺口与赛前清单 (别当"全家桶已齐"用)
缺口按优先级: 网络流 Dinic/MCMF(最优先, 直接吃 Graph::rev 红利) |
  字符串(ACAM/SAM/SA) | 计算几何 | 数学和 DP 偏薄(数学 10 件 DP 2 件)。
赛前清单: ① 同版 GCC 全库冒烟编译(-std=c++20, 基线 82 件零警告)
  ② run_checks.ps1 全量回归 ③ 确认评测机 GCC 版本、栈宽、内存限制
  ④ 按题核对预算口径(累计 4e6 / 峰值 1e6)够不够。

## 12. 终端执行规范 (违反 = 会话锁死或产出污染)
12.1 禁止交互式命令: less/more/vim/nano/top 这类要人按键才能退的
     一律禁。AI 会话的 shell 没有 stdin 可喂, 进交互态只能人工杀进程。
12.2 git 输出禁分页: git log/diff/show 一律写 git --no-pager <子命令>。
     不赌 core.pager 配置, 默认 pager 弹出来就是锁死(踩过)。
12.3 看文件用 cat/head -n/tail -n 或读文件工具, 不用任何会接管
     终端的编辑器。
12.4 Windows 命令行:
     - 连接符看宿主: cmd/PS7+ 用 &&(失败即停); PS 5.1 的 && 是语法
       错误, 只能用 ; 或拆开多次调用。不确定宿主版本就别写 &&。
     - 路径带空格/中文必须双引号(F:\c++\my_code\... 不加引号会被
       空格截断)。
     - PS5.1 没有 < 重定向(保留字), 要给程序喂 stdin 文件, 用
       cmd /c 包裹。
     - PS5.1 对程序的 > 重定向默认写 UTF-16LE+BOM+CRLF, 再喂回给
       程序读必炸(\0 会把数字切碎; i128 对拍那次假故障, -1 读成 0,
       一度冤枉了模板库)。要字节保真, 一律
       cmd /c "exe < in > out" 整体包裹, 别让 PS 碰字节流。
12.5 git 写操作要授权: push 等用户明说; 说了一次 push, 就可以
     git add . + commit + push 一气呵成, 不用再分步请示。零碎小改
     (rule 措辞类)攒在工作区, 跟下次实质改动合推, 不单独开 commit。
     (教训: §12 入宪那次, 没经授权 add/commit/push 三连, 被用户封堵。)
12.6 commit 信息要像人写的: 一行动词+对象。禁破折号轰炸、禁罗列、
     禁中英混排装饰、禁自造压缩词和营销腔("五查""直达"这种词是
     AI 发明的, 人只会说"加xx检查""修xx")。
     对: "补终端执行规范"。错: "rule: §12 终端执行规范——禁交互式
     命令/git --no-pager/..."。
12.7 命令超时或输出捕获失败时: 副作用可能只生效了一半, 恢复后
     必须先对账实际状态(数文件、抽内容)再继续, 禁止假设成功也禁止
     假设失败(实测: 删旧跳板生效了, 重新生成没跑, 留下 30 处烂账)。
     终端输出彻底读不到时, 改走日志中转: cmd /c 把命令输出落日志
     文件, 再用读文件工具看——绕开终端捕获层。

## 13. 刷题工作流 (zoi 全套, 已建成并全链路验证)
日常循环: cph 建题+爬样例 → 写短名 include(补全) → cph 测样例 → Ctrl+S
  → Ctrl+Shift+B 原地展开+复制剪贴板 → 提交 → WA 则命令面板 zoi-restore
  回紧凑态改 → 收工批量回溯。
换机部署: scripts\install-zoi.ps1 一条命令(用户级 settings 写 includePath
  + cph -I, 顺带跑 make_stubs 存状态); uninstall 按状态可逆撤除。
scripts\: 全部脚本在此, 根目录只留 rule/清单/README/.gitignore/
  .clinerules\(zoi.md 指针路由到 rule.md); 脚本里
  库根一律 Split-Path -Parent $PSScriptRoot。
CI(GitHub Actions): push/PR 即跑 ci.yml(ubuntu+g+++pwsh): run_checks 全量
  回归 + 全 cpp 语法扫(_check 除外, TEST GAP 盲区进网; misc 的 rw 仅
  Windows)。本地绿是义务, CI 红当场修; 结论匿名轮询 actions/runs API,
  日志要 token 故 WSL 复现通道是主力。2026/9/3 建成。
赛场纸质化: make_booklet.ps1 -> zoi-booklet.pdf(typst A4 横排三栏, 目录带页码);
  catalog 顺序即章节序, 行首 ^ = 笔记条目(.txt 正文, 无代码无跳板); 相对
  include 改写为跳板短名(誊写产物=同目录 .h 集合, utils 只印一次); 插件
  附录自动收(含 main 跳过); 每条目印 行数+ SHA256 前 8 hex(LF 归一化),
  自检先数行再对 hash; 纯 =/- 装饰线超栏宽转换期截断(先截后 hash, 纸
  面与指纹一致); 目录两级=域/条目,
  子域是分隔条(subsep), 未代表组的 README 印作导语(subintro); 全库知
  识点文件夹构建期对账(175 个), 缺条 exit 1 并打印缺席清单(防缺斤少
  两); 缺失叶子自动成骨架条目(README 作正文, 空壳打「待补」)紧凑连
  排, 真实条目一条一页; 大条目正面起排改 -SoloMin 可选(默认 0 连续
  排版零空白页, 双面打印场景 -SoloMin 90; parity 审计仅在 >0 时跑);
  条件分页禁 context 读页码(反馈循环); catalog 域/子域必须连续, 乱序
  构建直接 throw(否则子域分隔条重复印); 审计锚点用 typst eval 取(query
  输出无 location, 旧 JSON 审计曾静默空匹配假绿); 页码=一面一页; PS5.1
  按 ANSI, 中文字面量一律码点拼接(「待补」曾乱码成「寰呰」); typst
  雷区: _ 是强调开关(中文文件名须转义), content 里 # 开代码(禁裸 #/[)。
  typst 单 exe 落 scripts\(gitignore); 生成物不进 git。2026/9/3 建成,
  2026/9/5 全库覆盖+两级目录改版。
zoi\ 跳板层: stub = 一行 #include 指向真身, 纯 ASCII 短名, 中文路径只在
  zoi\_catalog.txt 出现; 新引擎入库 = catalog 加一行 + 跑 make_stubs。
  后缀必须 .h(.cpp 不进补全候选); 命名标准缩写留任、冷门驼峰全称;
  make_stubs 顺手盖 '// zoi: 名' 戳(预检校验一致); ! 豁免行 = 不配跳板
  的决策留痕, 预检做全覆盖校验; 自清洁删改名残留, 纯大小写改名 NTFS
  覆盖写不换名, 要先手删旧件。

scripts\zoi.ps1: expand <file> 备份 A.zoi.cpp 并把递归展开(内联本地
  include + 钻石去重 + 保留守卫)原地写回, 顺带复制剪贴板; restore <dir>
  批量回溯; status 干跑。
接口: Ctrl+Shift+B = zoi-expand; 命令面板 zoi-restore。cph 带
  -I<库根>\zoi(user settings), 直编与展开态共用短 include; IntelliSense
  已接(includePath 含 zoi): 补全/悬停/F12 穿透跳板均可用。
安全轨: 备份已存在拒绝二次展开; 无本地 include 则 no-op; 回溯前 SHA1
  内容指纹比对(内容没变放行, touch/自动保存不改内容; 真变了跳过警告,
  -Force 丢弃); 编辑器未保存先 Ctrl+S。
动机: 展开态原地写入 = 被测与提交同一份且零依赖库路径, 紧凑备份保赛后
  模板视角。坑史(mtime→SHA1 判据演进)详版见 history。
迁移 runbook(换机半小时): ① cpptools+cph ② settings 加 includePath +
  cph -I ③ user tasks 指 zoi.ps1 ④ clone ⑤ run_checks 绿 + expand 验证。
  脚本零硬编码路径; cph 只管建题/测面板; PS5.1 防御写法(纯 ASCII、
  cmd /c), PS7 兼容。

