# 清扫状态板

本表记录已经做过的具体工作，不代替 zoi/_catalog.txt 现役目录，也不宣称全库所有 API 已审完。
勾选仅对本行范围、工作和验证环境有效；源码或依赖改变后，旧结论留作历史，新开受影响项。
协作约定见 [collab.md](collab.md)，GLM 初始代记完整保存在 [sweep-history.md](sweep-history.md)。

## 当前领取

无。astra 已完成本轮收尾；下一次先领取下面一个单独范围，不自动开启全库扫描。

## 模板清扫记录（astra，2026-09-05）

| 状态 | 范围 | 本次工作 | 验证套件 |
|---|---|---|---|
| [x] | 整数开方 | 修复 long long 上界乘法溢出 | number_boundary |
| [x] | 区间筛质数 | 修复小区间 log(1) 与端点运算 | number_boundary |
| [x] | 约数个数及约数和 | 修复 int 上界试除、约数和溢出 | number_boundary |
| [x] | 可持久化字典树 | 补哨兵预留，修正复制根的预算与注释 | trie、trie_capacity |
| [x] | 字典树 | 构造参数、字符集和累计容量注释 | trie |
| [x] | 树状数组 | 构造/复位容量注释，确定性边界 | bit |
| [x] | 二维树状数组 | 行列上限与内存注释，确定性边界 | bit |
| [x] | 泛型线段树 | 用途、构造、重建与查询接口注释 | seg |
| [x] | 单调队列 | 修复 init 清空 vector 后下标写入；保留无状态复位入口 | misc；Linux sanitizer + Windows PS5.1 |

## 对拍执行记录（astra，2026-09-05）

Linux GCC 13：普通回归 18/18；初次 sanitizer 17/18，misc 的单调队列越界修复后定点通过。
以下是已执行记录，不把“运行通过”写成“已逐行审查全部接口”。新增/补强内容单独列出。

| 状态 | 套件 | 新增或补强 | 已验证环境 |
|---|---|---|---|
| [x] | seggraph_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | graph_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | hld_check | 单点、链、星、平衡树 | Linux O2 + ASan/UBSan |
| [x] | vt_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | conn_check | 仅纠正割点暴力注释；原行为测试保持 | Linux O2 + ASan/UBSan |
| [x] | trie_capacity_check | 新增：恰好顶满预算、空串根复制、clear 复用 | Linux O2 + ASan/UBSan |
| [x] | trie_check | 空串、62 字符集、LL 高位、重复值、空差集与 kth 边界 | Linux O2 + ASan/UBSan |
| [x] | mint_check | 全64位随机运算数、MOD-1；避免构造后对最小负数取反 | Linux O2 + ASan/UBSan |
| [x] | number_boundary_check | 新增：整数极值与 1000 组独立暴力；重复 include 是守卫测试 | Linux O2 + ASan/UBSan |
| [x] | bigint_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | leftist_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | dsu_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | oset_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | bit_check | 满容量、首尾、单行单列、1e12、大—小—大复位 | Linux O2 + ASan/UBSan |
| [x] | pst_check | 任意历史版本分叉 | Linux O2 + ASan/UBSan |
| [x] | seg_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |
| [x] | misc_check | 单调队列复位反例；跨平台、4MiB 边界、整数极值与独立字节比对 | Linux O2 + ASan/UBSan；Windows PS5.1 O2 |
| [x] | rnd_check | 仅运行已有回归 | Linux O2 + ASan/UBSan |

## 规则与测试入口

- [x] rule.md 分卷：原 365 个非空行逐行对账零缺失；旧执行条款归历史。
- [x] 协作约定：GLM 铺底、astra 审查收尾；一次一个范围；同文件避让；以证据打勾。
- [x] 回归入口：零匹配失败、Werror、双超时、独立目录、日志和 summary；9 项自检已在 Linux/Windows PS5.1 通过。
- [x] Linux 普通回归与语法扫描：107/107（18 套件 + 89 cpp）。
- [x] CI 配置：双作业、失败仍语法扫描、上传日志；actionlint 通过。
- [ ] 修改后的 CI 在线运行：尚未提交/推送，不能用旧提交的绿灯代替。
- [ ] Windows 全库扫描：本轮仅定点验证 misc 与入口，未重复全库。

证据：项目 .ci-results/misc-fix、.ci-results/misc-windows 的 summary.json；
此前全量日志位于本次 Codex 工作目录 work/linux-release、work/linux-sanitizer、work/linux-runner.log。
Windows 入口自检为 work/runner-windows-fixed.log。日志可清理，结论保留在本板。
未提交时填验证结论即可，不为补提交号擅自 commit。

## 待办池（下一次只领一行）

- [ ] 线段树合并分裂：先选三件中的一件，补独立暴力。
- [ ] 线段树套 FHQ：对拍。
- [ ] 树上背包：先选一种实现，对拍。
- [ ] 备用 LCA：先选一种实现，对拍。
- [ ] 数学轻件：按实际使用选一件补边界，不为消灭 TEST GAP 硬凑测试。
- [ ] 网络流 Dinic：先核实当前资产，再独立入库/对拍；MCMF 另开一件。
- [ ] AC 自动机：先核实当前是笔记还是引擎，再决定入库或对拍。
- [ ] 后缀数组、后缀自动机、计算几何：各自另领单独范围。
- [ ] SCC/连通性注释：GLM 标记存在语义分歧，astra 未动，保留用户裁决入口。
- [ ] 连通性 README 删除后的手册：沿用 GLM 后续验证，不在本轮重建。

TEST GAP 是无直接引用提示，不是零行为覆盖证明；utils/i128 可能间接使用。
number_boundary 的重复 include 有意验证守卫，不作为“误粘贴待删”。
