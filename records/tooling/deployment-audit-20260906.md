# 2026-09-06 队友部署、VS Code 任务与缓存审查

本轮处理队友实遇的安装、补全和任务问题；未推进 P10，保留此前 P05–P09 工作区改动。现行操作以 [安装指南](../../docs/setup/README.md) 和 [维护说明](../../docs/maintenance/README.md) 为准。

## 已确认问题与修复

| 问题 | 处理与验收边界 |
|---|---|
| 仓库跟踪了维护者的 scripts/.zoi-install-state.json，干净克隆也报 Legacy install state | 原文备份到忽略目录 docs/backups，移除跟踪文件，Git 与 ZIP 排除个人状态；未知旧安装记录仍不猜删 |
| 用户级 includePath 被刷题目录的 C/C++ 配置覆盖；原设置还关闭了补全 | 安装器启用用户默认值；zoi-configure 显式补当前文件夹每个 C/C++ 配置并保留原 include/defines/compiler，支持快照撤销；doctor 只读显示相关配置 |
| 重装原先立即返回，无法补任务、升级配置 | v3 管理 settings/tasks/keybindings 三份文档，兼容 v2 两文档状态；重复安装补缺项且保留最初快照，三个写入边界均做故障恢复测试 |
| 手写旧任务与新安装冲突 | 显式 -AdoptExistingTasks 只接管当前库对应脚本的已知任务；外部命令拒绝，卸载恢复原任务并保留后来新增的其他任务 |
| Ctrl+Shift+P 的排序不构成固定任务入口 | 注册 Ctrl+Alt+T 打开 task zoi-，Ctrl+Alt+Z 展开当前 C++；已有同键绑定保留，另提供 Ctrl+P 手输入口；不篡改 VS Code 命令历史 |
| 展开 0 blocks 没有解释，已知短名尖括号被当成标准头跳过 | 增加零块提示；已知 <name.h> 直接从 zoi 解析，标准头不变；同名源目录头不能遮蔽尖括号跳板，恢复保留原 include 拼写 |
| ZIP 只有手动路径，没有正式任务入口 | zoi-package 注册为用户级 process task，默认 docs/releases/时间戳.zip，不依赖活动编辑器文件；已有文件不覆盖，完成压缩后才发布正式文件名 |
| 打包过滤用绝对路径，在 .zoi-checks 内解压再打包时把所有源文件排除了 | 改为相对库根过滤，测试真实 ZIP 解压→再打包→再解压→全新配置安装/卸载；禁止链接进入包 |
| 排除 releases/backups 时连 README 指南也漏掉 | 两个目录只显式收 README，实际 ZIP/私人备份不收；部署自检确认指南存在和隐私产物不存在 |
| Windows 暂时锁住 stage，合法 ZIP 已生成却报整体失败 | 有界重试清理，仅保留仍被占用的临时 stage 并给出路径，不把有效 ZIP 误报失败 |
| 测试目录无限积累，双 PowerShell 同时结束会争删旧缓存 | 完成标记记录 PASS/FAIL、类型、绝对路径；只清理路径吻合的旧成功目录，每类留三份；独占锁避免并发清理；失败、未知、坏标记、路径不符、链接及人工工作区保留 |

安装注册九任务：expand、restore、restore-all、status、forget、configure、doctor、package、clean-checks（均有 zoi- 前缀）。用户级真实配置已按授权升级，原 zoi-booklet 任务保留；复核状态为 v3/installed、三份原始快照、九个受管标签与两个快捷键。

## 验证

- Windows PowerShell 5.1 与 PowerShell 7：check_setup 各 7 组 / 28 命令通过，包括 JSONC/BOM/CRLF、原文恢复、三文档中断恢复、改动后保留、受管包删除边界。
- 两种 PowerShell：check_deployment 各 20 命令通过，覆盖 v2 升级、任务修复、快捷键冲突、显式接管、中文/空格/# 刷题目录、多配置 includePath、撤销冲突、真实 ZIP 交付与缓存预览/并发锁/保留策略。
- zoi_check：Windows PS5.1、PS7 及 Linux PowerShell 7.4.13/GCC 13 通过原 17 组和新增尖括号往返编译用例；最终统计为 18 组 / 94 命令。早期日志的汇总行在新增用例前，已移到所有用例之后。
- Windows run_checks -Mode All：112 项，0 失败；包含全库回归和语法/警告扫描，报告 .ci-results/setup-audit-final/summary.json，算法证据由原 runner 正常登记。
- Linux check_runner：9 个故障路径自检通过。日志中的预期 FAIL 属于注入失败的断言场景，不代表总套件失败。
- PS5.1/PS7 的 reliability、features、verification 生成表检查通过；actionlint 1.7.12 和 git diff --check 通过。
- .github/workflows/ci.yml 的 Windows setup 作业已自动执行两种 PowerShell 的新增部署套件；原 Linux 展开、入口故障、普通回归、压力与 sanitizer 作业保留。尚未提交/推送，因此不声称本轮远程 CI 已运行。

原始日志在 .zoi-checks/codex-work/setup-audit，后续可以按保留规则清理测试副本，本记录保留环境、范围和结果。

## 缓存处理

开始时 .zoi-checks 为 147.73 MiB、5976 个文件、140 个直属目录。它保存测试二进制、临时输入、隔离配置和失败现场，正常 include、展开和刷题不依赖它。

本轮将 115 个旧无标记测试目录的 3317 个文件（10,604,466 字节）先归档，逐文件核对 ZIP 与原文件 SHA256，再删除这些已归档目录。归档为 docs/backups/zoi-checks-legacy-20260906-234818.zip（4,983,771 字节），SHA256：

`D18632F831FEC028116C75650B9580E775ECAFB46C36A2D3878D953006E64AD4`

同时移除已使用完的 pwsh-linux.tar.gz（72,482,320 字节）、actionlint.zip（2,479,065 字节）和本轮解压的临时 PowerShell 运行时（184,921,430 字节）；保留可用 actionlint 与人工研究现场。临时运行时是本轮新解压产物，其清理量不能算作相对初始占用的净下降。

新成功缓存限量滚动，旧未知和失败现场不自动认领；Windows/WSL 各自按可验证的绝对路径清理，不跨平台猜映射。正式 ZIP、归档备份与 .ci-results 不参与自动清理。并非清空整个目录，也未清理 VS Code/WSL 自己的 C 盘缓存。

## 同步与实际使用限制

rule、用户功能表、安装/展开/维护/发布指南、文档导航、测试说明与工作板已同步。生成算法目录无关系变化，无需改写；本轮不涉及算法学习状态。

安装器不安装插件或编译器，队友仍需启用当前 Profile 的 Microsoft C/C++，打开刷题文件夹后运行 zoi-configure。configurationProvider、compile_commands、语言专属和多根工作区设置仍可能覆盖普通配置，诊断提示后应调整其实际来源。用户本机的配置文件已核验，未操作队友机器的编辑器 UI，不能把脚本自检等同于对方 UI 已验收。
