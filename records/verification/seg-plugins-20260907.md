# 七组线段树代数插件

2026-09-07，用户要求约七组代表性的 Info/Tag，不枚举操作与信息的笛卡尔积；确认配简短选型 README，这批七组均学过待复习。本轮未提交推送。

## 范围与接口

原“区间加区间和”升级为 SegAdd（和/最小/最大），新增 SegAssignAdd、SegAffine、SegXor、SegBinary、SegMaxSubarray、SegLinear 六组。七份源码各有守卫、独立命名空间、utils 相对引用和完整 Usage；不再用普通头或全局 Info/Tag 制造冲突。旧加和插件类型从全局 Info/Tag 改为 SegAdd::Info/Tag，检索未发现库内调用者。普通与动态线段树引擎未改。

现有历史最值、取模、主席树插件及 P2572 题解保留，本报告不声明它们通过本轮验收。七组适配与限制以 [选型说明](../../algorithms/数据结构/线段树/泛型插件/README.md) 为准：Binary/Linear 必须完整 build，不能仅补 len 构造虚区间；其余五组支持零域稀疏动态树。Xor 支持 1..31 位、长度不超 INT_MAX，仿射模数为 2..INT_MAX，其余普通整数及累积标记中间值须在 LL 内。

## 测试与结果

新增 `seg_plugins_check.cpp`，直接 include 七份真实代数及两份引擎，包含重复 include，未复制一套代数冒充测试。

- 每组 64 轮×320 次普通/动态树同步更新查询；小数组逐元素更新，直接扫描 sum/min/max、位计数/异或、01 前后缀/最长段、位置和。最大子段暴力枚举全部子段，仿射独立参照用 __int128。每轮末尾枚举全部查询区间。
- n=1、全同负值、交替值、空单位元/空 Tag、赋值 0、整段连续操作后局部下传与单点、模数 INT_MAX 和负系数、异或第 30 位。五组从未 build 的 1e9 零域进行后缀修改、未触及区间查询、clear。
- 每组以同一普通树跑 200000→1→257→200000，初始化交替 ±1e9/高位/01；每轮 20 次长区间操作和 n 次单点/短区间交错更新查询，最后全域独立扫描。共七组依次运行，避免并行驻留大结构。最大子段大域参照使用前缀最小值。
- Windows GCC 15.2.0，C++20 / O2 / Wall Wextra Werror / assert 启用，全部通过。runner 编译与执行共 12.722 秒；结构化证据为 [bafe618cbd1a4730b20f46b51d2c0557](runs/bafe618cbd1a4730b20f46b51d2c0557.json)，覆盖当前七份源码和最终测试指纹。
- WSL Ubuntu 20.04 的 GCC 13.1.0，O1 / ASan+UBSan / no-recover / 标准库断言 / leak 检查，最终同一套默认用例全部通过、退出 0。默认 GCC 9 不支持 C++20，编译探测失败后改用机器已有 GCC 13；没有降低语言标准。此项为专项实际运行记录，不伪造成 runner 的 Linux JSON。
- 七份 Usage 分别提取、编译、运行，输出与注释完全一致：10/2、-2、15/83、6/0/2、2/2、6/-5、12。

`rules/verification.json` 登记两份引擎与新套件的行为范围；两层表自动生成。插件仍是代数示例身份，不强制创建独立跳板。新增 include 关系同步 reliability，源码数量变化同步 features。范围登记整文件参与旧记录指纹，未运行旧套件不会伪继承旧绿灯。默认 runner 自动发现新 *_check.cpp，push/PR 普通和 sanitizer 作业都会执行，无需在 CI 写套件名单。

## 文档与纸质化

规则、功能表、测试指南、既有线段树 README、七组学习记录和设计分类已同步；全库分类从 213 到 219 项，数据结构从 39 到 45 项，新增六项均为固定内核加题目骨架。没有替旧专题推断学习状态。

插件扫描原来直接搜 `int main`，会误排除 Usage 注释中的 main。现先剥离注释与普通字符串/字符字面量，再判断真实主函数；新增“注释 main 保留 / 真实 main 排除”样例。带中文目录的重定向子进程测试还发现 Typst 的 UTF-8 输出被按本机编码读入，现审计期间显式 UTF-8 解码并恢复原控制台编码。PS 7 Render、PS 5.1 SourceOnly 各 17 次构建通过；增长样例 17 页、17 个目录、4 个实现、1200 行与七层层级检查通过。

正式 PDF 自动收录七组和选型说明，138 页、187 个算法目录、103 个独立页段、13 份 README、52 个数学节点、274 个目录条目。逐实现预留与第 39 页 MIKU 保留，正文/目录/页眉和说明文本自动检查通过。正式 PDF SHA256 为 `55d482a8c0abeef4b2e3a5e62c88a8054785ef3c8ca9c0bf292701c075c8db45`。
实际渲染抽查仿射代码页和选型说明页：代码完整、独立起页留白保留，表格和公式未溢出或重叠。文档双入口 106/106 可达，219 项分类检查、features/reliability -Check、diff --check 均通过。

现场：`.zoi-checks/codex-work/seg-plugins/`，保留 windows.log、linux.log、usage.log、booklet.log、pdf-check.log 及渲染图；Windows runner 原始日志在 `.zoi-checks/eabb8be326a643e28e6d55d450ed4645/logs`。成功 runner 与 booklet 样例继续按现有保留策略清理。尚未推送，线上 CI 本轮未运行。
