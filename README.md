# HNIST-ACM-2024

面向 ICPC 的 C++20 模板库：短名 include 组装算法，对拍后展开成提交源码，也能生成纸质比赛手册。

**[库里有什么](docs/features/README.md)** · **[查算法和跳板](docs/features/catalog.md)** · **[使用指南](docs/README.md)** · **[学习进度](docs/progress/README.md)**

初次使用：[安装](docs/setup/README.md) → include 所需模板并写题 → [展开提交](docs/expand/README.md)。
维护者：[对拍](scripts/checks.md) · [脚本维护](docs/maintenance/README.md) · [AI 入口](AGENTS.md)。

笔记和待补内容不等于可用引擎；具体接口、内存预算看源码，运行结果看当次报告。

历史方案与证据：[records/tooling](records/tooling/README.md)。

## 仓库目录

```text
HNIST-ACM-2024/
├─ algorithms/       算法母版、笔记、例题与各家族的对拍
├─ zoi/              短名头文件跳板；_catalog.txt 是登记清单
├─ scripts/          安装卸载、展开恢复、对拍、目录生成与纸质化脚本
├─ docs/             给使用者看的中文指南
│  ├─ features/      功能总览、算法与跳板目录
│  ├─ progress/      本人的学习进度与模板整理状态
│  ├─ setup/         队友安装与卸载
│  ├─ expand/        展开提交、恢复及状态清理
│  ├─ booklet/       打印指南；output/ 放 PDF 与排版源文件（不提交）
│  ├─ maintenance/   修改脚本、维护目录和制作分享包
│  ├─ releases/      本地生成的队友 ZIP（产物不提交）
│  └─ backups/       个人恢复备份（备份不提交）
├─ rules/            协作约定、测试坑位、工作板与测试资产表
├─ records/          训练复盘与历史；tooling/ 保存旧规则和工具改造证据
├─ contests/         比赛相关资料
├─ .github/          GitHub Actions 自动检查
├─ .clinerules/      Cline 入口，统一指向 AGENTS.md
├─ .vscode/          仓库级编辑器配置
├─ AGENTS.md         AI 按任务选择规则的入口
├─ rule.md           算法模板开发契约
├─ .zoi-checks/      隔离测试、临时工作文件（不提交）
└─ .ci-results/      测试汇总与日志（不提交）
```

写题看 `algorithms/` 和 `zoi/`；使用工具看 `docs/`；AI 接手先看 `AGENTS.md`。生成物和临时目录运行后才会出现，不需要手工创建。

*filled with determination*

原有参考：[超时问题解决](https://blog.csdn.net/weixin_44223180/article/details/133059575)。
