# 清扫历史代记

以下为 GLM 建板及 astra 接手时的原文快照，保留追溯；其中勾选与锁定不代表当前状态，当前结论只看 sweep-board.md。

# 清扫状态板 (token 有限者的外挂记忆; 用法见 rules/collab.md)

打勾纪律: ☐ 待办 → 🔒 进行中(署名) → ✅ 完成(记提交号或结论);
发现问题开 ❌ 行。一次只做一个范围内的一件事。

## astra 2026/9/5 工业化大扫除 — 历史进度(种子状态由 GLM 依侦察代记,
astra 接手后自行更正补漏)

- [x] 回归入口重写: run_checks -Mode/-Sanitize/双超时(进程树)/summary
      报告/.zoi-checks 独立构建目录
- [x] 入口自检器 check_runner.ps1 + check_process.ps1(八种故障注入,
      验证入口不假绿)
- [x] CI 双作业(回归 + sanitizer), 失败仍跑语法扫并上传日志
- [x] 宪法分卷: rules/pitfalls.md(坑位表整迁) + rules/workflow.md
      (§10/13 整迁) + scripts/checks.md + rule.md 阅读顺序路由
- [x] 引擎修复: 区间筛质数(乘法溢出 + 浮点 sqrt→floor_isqrt + 守卫 +
      Usage) | 整数开方 | 约数个数及约数和公式法 | 字典树(构造契约注释)
      | 可持久化字典树 | 树状数组 + 二维 | 泛型线段树
- [x] 新对拍: trie_capacity_check | number_boundary_check;
      misc_check 大改; bit/pst/hld/conn/mint/trie 适配 -Werror
- [ ] -Mode All 全库 -Werror 终态未验证(GLM 后台发射的验证无输出)
- [ ] number_boundary_check.cpp 头部三件套 include 重复粘贴两遍
      (守卫化了, 无害待清)
- [ ] rule.md 头部 L9/L10 归档句重复; "最近一轮"未更新为本轮
- [ ] ❌ 分歧待裁: SCC 等 conn 四件注释二改删掉了鸭子协议语义
      (SegGraph 带权图直喂 / 输出恒无权), 与 GLM 精炼版并存两案
- [ ] 连通性相关/README.md 已删: booklet 组导语 10→9, 手册重建待验证
- [x] 对拍审查报告(原 对拍清单.md 头部 +18 行, 已由 GLM 核对并入本板;
      原文件经用户裁决删除, 使命由 run_checks 输出与 summary.json 取代):
  - 补齐: 树状数组(满容量/单行单列/首尾更新/1e12 量级/大-小-大复位) |
    HLD(单点/满容量链/星形/平衡树, 复用爬父暴力) | 字典树(空串/62 字符
    集/LLONG_MAX/第 62 位/重复值/空差集/kth 越界约定) | 主席树(任意历史
    版本分叉修改再随机回访, 原来只改最新版) | mint(64 位随机源+MOD-1,
    128 位随机避开最小负数取反) | 快读写(64/128 位极值/EOF/科学计数法/
    无终止符/跨 4MiB 缓冲/跨平台字节比对) | 入口(八种故障必失败+正常用
    例防"一律失败"伪自检)
  - 缺口如实保留: 线段树合并分裂三件 / 线段树套 FHQ / 树上背包两件 /
    备用 LCA 三件, 待实际使用时补独立暴力; utils/i128 可能被间接引用
    不判零覆盖; 轻件不为消灭黄牌强塞测试
  - 确认并修复的原入口五病: 拼错 Filter 假成功 | 无超时 | 警告不阻断
    (现 -Werror) | 回归失败跳过语法扫 | Linux 不跑快读写

## 当前领取

- [ ] 🔒 astra：单调队列 init 越界修复与 misc 定点验证；涉及单调队列.cpp、misc_check.cpp。

## 待办池(善后者按范围领, 一次一件; 领取时改 🔒 署名)

- [ ] 网络流: Dinic + MCMF 入库(§11 最优先缺口), 对拍暴力 = BFS 增广
- [ ] AC自动机: 对拍(暴力 = 逐模式串 KMP 匹配)
- [ ] 后缀数组 / 后缀自动机: 对拍(暴力 = 后缀排序 / 逐子串)
- [ ] 计算几何: 凸包 / 半平面交 / 二维基础 边界对拍
- [ ] TEST GAP 22 件无直接对拍引擎, 按域分批补(先数学数论系)

## 铺底日志(GLM 日常, 简记)

- 2026/9/6: 搭建本协作体系(AGENTS.md / collab.md / sweep-board.md),
      种子状态录入; astra 五小时大扫除期间未动其活跃区。