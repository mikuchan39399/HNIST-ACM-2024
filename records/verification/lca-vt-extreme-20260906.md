# LCA 与虚树极限验证 (2026-09-06)

范围: DFN_LCA、HLD_LCA、二次排序虚树、单调栈虚树。用户要求先极限对拍再润色注释, 本次补齐之前只到 50/64 点的缺口, 不修改四份引擎或继续润色注释。

测试源码: [lca_vt_stress_check.cpp](../../algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp)。新套件由现有 runner/CI 自动发现, 不另造测试入口。原 graph_check/vt_check 保留。

## 默认回归

- mt19937(42), 400 组最多 40 点带权树/森林, 穷举每组全部点对, 独立父表爬链核对 LCA、距离与路径跳转; 200 组连通树另用 BFS 重定根, 检查 HLD 指定根和两种虚树
- 检查空点集、重复点、跨分量、空森林, dep/dis/rt/sz/dfn/父表及 DFN rnk; 不声称完整 rmq/fa 倍增表或 son/top 已逐项核对
- 3 个固定三点星, 权分别为 0、LL_MAX / 2、LL_MIN / 2, 覆盖逼近 LL 两端的路径权和, 所有中间运算仍在契约内
- 20 万点星、二叉树、随机树、64 点链块组成的森林, 每种 10 万次点对查询
- 两份 LCA 分别搭配两份虚树, 核对全点、2n 长重复输入、空集、单点、稀疏点集、非祖先必选 root、跨分量和大-小-大复用
- 虚树参照不调用 LCA, 不使用 DFS 序或单调栈: 从独立父树剪除不含必选点的分支, 保留必选点和至少两个有效儿子的分叉点, 连到最近保留祖先

Windows GCC 15.2 普通回归与 Linux GCC 13.1.0 ASan/UBSan 均通过。Windows runner 的结构化证据自动写入 records/verification/runs, 最终日志在 `.ci-results/p03-extreme-final/lca_vt_stress_check`。Linux 是手动运行, 不伪造 runner JSON, 自动表只标结构化 Windows 结果。

## 手动规模与栈探针

| 环境和范围 | 结果 |
|---|---|
| Windows GCC 15.2, `--large 1000000`, 四种浅树形态及四种组合 | 通过 |
| Linux GCC 13.1.0, `--large 1000000`, 四种浅树形态及四种组合 | 通过 |
| Windows 默认 PE 栈预留 2 MiB, DFN/HLD 各自 `--probe-* 200000` | 两者均失败, 退出码 -1073741571 (0xC00000FD) |
| Linux 栈限制 8 MiB, DFN/HLD 各自 `--probe-* 200000` | 两者均失败, SIGSEGV, shell 退出码 139 |
| Linux 栈限制 256 MiB, `--deep 1000000`, 四种组合 | 通过 |

20 万点深链是实测失败点, 不是精确的最大安全深度。默认回归采用浅树, 不把栈探针失败隐藏在全绿结果里。增栈仅作用于测试子进程, 不改变用户的系统配置; 不能据此宣称默认栈能处理百万深链。

用户明确选择保留递归、记录实际限制; 未将两份 LCA 改为迭代。百万规模的耗时和峰值内存在原始 time 日志, 峰值包含两份 LCA、两份虚树、原图和独立参照, 不能当成单个模板的内存账目。

## 复现

默认带证据入口:

```powershell
./scripts/run_checks.ps1 -Filter lca_vt_stress_check
```

Linux 普通压力:

```bash
g++-13 -std=c++20 -Wall -Wextra -Werror -UNDEBUG -O2 \
  algorithms/图论/树上问题/虚树/对拍/lca_vt_stress_check.cpp -o /tmp/lca-vt-stress
/tmp/lca-vt-stress --large 1000000
(ulimit -c 0; ulimit -s 8192; /tmp/lca-vt-stress --probe-dfn 200000)
(ulimit -c 0; ulimit -s 8192; /tmp/lca-vt-stress --probe-hld 200000)
(ulimit -s 262144; /tmp/lca-vt-stress --deep 1000000)
```

sanitizer 将普通编译的 `-O2` 换为 `-O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -D_GLIBCXX_ASSERTIONS`, 无参数运行默认套件。Linux sanitizer 原始日志 `.zoi-checks/codex-work/lca-vt-extreme-san.log`。

其余手动日志位于 `.zoi-checks/codex-work/`:

- `lca-vt-million-linux.log`、`lca-vt-million-windows.stdout.log`
- `lca-vt-default-dfn.log`、`lca-vt-default-hld.log`
- `lca-vt-default-windows-dfn-result.json`、`lca-vt-default-windows-hld-result.json`
- `lca-vt-deep-million-256mb-linux.log`

临时日志不提交; 本记录明确区分默认回归、手动压力与预期失败的栈探针。未运行本轮远程 CI, 未提交、未推送。

## 后续接入 CI (同日用户授权)

上述手动压力已通过 `scripts/check_lca_vt_extreme.py` 接入 CI 的 stress 作业, 随 push/PR 自动运行; 历史手动结果保留, 不改称线上通过。百万点常规结构与 256 MiB 栈深链必须通过, 默认 8 MiB 栈探针改用 ASan/UBSan 明确识别栈溢出, 不能把任意 SIGSEGV 视为允许的失败。

脚本的自动报告包含源码/依赖哈希、参数、逐项状态及日志, 由 stress artifact 上传; 不手造 runner 证据。完整入口已在本机 WSL 验证, 日志 `.ci-results/stress-lca-vt-local`; 线上作业仍须推送后实际运行。
