# 打印比赛手册

日常训练继续使用原来的头文件；纸质手册固定 **C++20 赛场版**，按算法一级目录各生成一册 A4 竖版单栏 PDF。

## 一键生成

在库根运行，或使用原来的 VS Code `zoi-booklet` 任务：

```powershell
./scripts/make_booklet.ps1
```

这条命令自动完成源码选择、九册排版、打印代码测试和 PDF 检查，通过后更新 `docs/booklet/output/chapters/`。不需要手调页码、单独维护纸版源码或逐章构建。编辑源码后需重新运行，已有 PDF 不会随编辑实时刷新。

每册有 `.pdf`、`.typ` 和 `.code.md`：前者打印，后两者用于核对排版输入和实际纸面代码。`chapters.json` 是当次完整清单；含 `validation` 字段才表示经过完整构建检查。可从 GitHub Actions 的 booklet 作业下载对应提交的这些产物。产物不提交 Git。

### 构建环境

支持 Windows PowerShell 5.1、PowerShell 7；需要支持 C++20 的 g++、Python 3 与 pypdf，以及 Typst 0.15.1（验证版本）。Typst 放 PATH 或 `scripts/typst.exe`。中文字体选 Noto Sans SC、Noto Sans CJK SC 或微软雅黑；西文选 Source Sans Pro 或 DejaVu Sans；代码选 Consolas 或 DejaVu Sans Mono。缺少必需环境会直接报错。

Python 可使用当前已安装 pypdf 的环境，也可以在库根准备独立环境：

```powershell
python -m venv .zoi-checks/booklet-tools/venv
./.zoi-checks/booklet-tools/venv/Scripts/python.exe -m pip install pypdf==6.0.0
```

构建优先使用这份库内环境，找不到才使用 PATH 的 python；Linux 的对应路径是 `venv/bin/python`。环境只需准备一次，不修改题目编译设置。不需要运行题目时安装这些 PDF 依赖。

需要指定工具或导出位置时用完整入口：

```powershell
./scripts/build_booklet.ps1 -Python <python路径> -Compiler <g++路径> -TypstPath <typst路径>
./scripts/build_booklet.ps1 -OutDir .zoi-checks/codex-work/contest-booklet
```

## 纸版怎样调用

KMP、Manacher、ZFunction 的纸版统一显式传 span，不印旧标准包装、string/vector 重载或内部输入转发层。三者与电子版共用算法循环；电子版原有调用和 C++11 起的兼容能力保留。

```cpp
string s = "abacaba", p = "aba";
KMP kmp{span(p)};
int pos = kmp.find_first(span(s));
Manacher man{span(s)};
ZFunction zf{span(s)};
VI e = ZFunction::extend(span(s), span(p));
VI a = {0, 7, 2, 7};
zf.build(span(a).subspan(1)); // 跳过占位
```

span 的每项都有效，结果位置相对片段，沿用对应模板的 1-based 约定。字符串变量用 `span(s)`，字符串字面量直接转 span 会包含末尾零字符。`vector<bool>` 不能直接转 span，需要时改用普通字节数组。KMP 复制模式；Manacher/Z 不保留输入，视图在调用期间有效即可。默认空构造可用，其他边界与复杂度看纸面短注释。

## 什么会印进去

- `algorithms/` 的真实目录决定章节，包括待建目录；现役实现、登记笔记、跳板名和已有家族顺序沿用 catalog。对拍与隐藏目录不入册。
- 同目录多份实现分别入册，代数插件回到所属目录；带 main 的完整题解不作为插件打印。
- 源文件同目录的 README 自动跟随最后一份选中的实现，只印一次；不继承父/子目录 README，不收路线长文。是否创建算法说明仍由用户决定，当前获准说明为线段树插件、树的直径、树的中心、树的重心。
- 私有实现头随所属条目展开；已登记公共依赖保留短名 include。源码内的纸版条件和语言版本条件自动选择，无需独立配方。

待建叶目录以 `.gitkeep` 保留供克隆与打包，它本身不打印。新增目录自动进入手册；新增模板按正常短名登记流程接入。

当前九册为数据结构、动态规划、图论、字符串、数学、计算几何、杂项、搜索、算法基础。各册有 ICPC 标志和章节名的简洁封面、独立目录和页码；每份实现另起页，说明跟随，页尾留作补写，长模板自动续页。代码 9 pt，注释深灰；页眉提供返回目录链接。待建目录集中排，达到第 39 页的册子保留 MIKU ♡ 页脚。

封面素材取自 [ICPC 官方 Press 页](https://news.icpc.global/press/)的 [Foundation SVG](https://news.icpc.global/icpc_foundation.svg)，保存于 `assets/icpc-foundation.svg` 并内嵌到排版源，生成和阅读不需要联网。

## 说明怎么排版

目录页和正文标题共用按深度递减的样式：方向最醒目，家族其次，子目录逐层减小字号、收敛字重与颜色；目录每层缩进 10 pt。深层只限制最小字号，不截断目录深度，不会退回更大的默认样式。目录行保留独立间距，避免紧凑排版挤叠文字。

四份获准说明统一采用用户确认的“树的中心”样式：深绿色主标题、编号小节、浅灰公式框、细横线表格和左侧竖线提示。颜色只作辅助，黑白打印也能通过字号、编号和线条区分层级。代码为 9 pt，说明正文 10 pt；代码使用深色高亮与深灰注释，方便黑白打印。目录引导点降低颜色与密度，让标题更突出；说明标题不进入算法目录。

源码 README 仍写普通 Markdown，按以下约定得到纸面样式：

| 写法 | 手册中的效果 |
|---|---|
| `# 标题` | 16 pt 深绿色标题，下面有细色线；每份说明的小节编号重新开始 |
| 主标题后第一段 `> 短名 · 适用条件` | 9 pt 灰色副标题，仅消费这一行；可用反引号写头文件名 |
| `## 小节` | 自动加 01、02 等编号，12 pt 标题和浅灰分隔线；源码不手填编号 |
| `### 小标题` 至六级标题 | 11 pt 小标题，不额外编号；各级说明标题都与后文保持相连 |
| 独立一行 `**公式条件或名称**`，随后为 `$$…$$` | 9 pt 标签与 11 pt 公式放在同一个浅灰框；中间可有空行 |
| 其他位置的 `> **易错** 内容` | 左侧深绿色竖线提示，连续引用行合成一块，可跨页续排 |
| 标准列表与表格 | 列表续行悬挂对齐；表格为等宽列、浅灰表头、细横线，跨页重复表头 |

单栏内优先使用简洁的两列表格，长注意事项移到对应段落或提示块。公式标签应写清适用条件，提示块用于边界与反例；不要把整份说明包成不可分页的大框。无副标题、无公式标签的现有 Markdown 也能直接排版。

例如，以下写法在仓库里仍可正常阅读，不需要手写 Typst 或为单份说明定制脚本：

```markdown
# 树的中心

> `center.h` · 无权 / 非负整数边权

## 无权树：取直径中点

**仅无权树可直接使用**

$$R=\left\lceil\frac{D}{2}\right\rceil$$

> **易错** 剥叶不能照搬到带权树。
```

入册说明面向赛场：写适用条件、关键性质、公式、接口、修改位置和易错边界。学习路线、对拍/CI 记录、维护与迁移历史留在仓库的对应文档里，不混进纸面速查。

公式用 `$x_i^2$` 写在行内，用 `$$…$$` 单独成块，也支持将两个 `$$` 各放一行。PDF 使用 New Computer Modern Math 数学字体，独立公式为 11 pt、浅灰底，分式、上下标与求和上下限按数学结构排版。反引号和代码围栏内的美元符号保持原文。

支持常用 TeX 子集：`_`、`^`、`\frac{a}{b}`、`\sqrt{x}`、`\sum`/`\prod`、`\min`/`\max`、`\log`/`\ln`、关系与箭头，以及 `\operatorname{lca}`、`\mathrm{ecc}`、`\mathcal{C}`。`\left`/`\right` 可配圆括号、方括号、绝对值或取整符号，按内容高度伸缩。可用命令以转换器白名单为准；不支持宏、矩阵或完整 LaTeX 环境，未知命令、错误括号或未闭合公式直接报错。新增写法先筛选预览，不能靠构建成功代替检查公式含义。

纸上链接显示可读标题，完整目标保留在仓库 Markdown。不执行嵌入 HTML、图片或 Typst 代码；复杂 Markdown 的原始符号可能按文字保留。未闭合围栏、表格列数不一致直接报错，避免静默漏内容。

## 筛选预览与打印

```powershell
./scripts/make_booklet.ps1 -Filter kmp -OutFile .zoi-checks/codex-work/kmp.pdf
./scripts/make_booklet.ps1 -Chapter 字符串 -OutFile .zoi-checks/codex-work/字符串.pdf
./scripts/make_booklet.ps1 -SourceOnly -OutDir .zoi-checks/codex-work/booklet-source
```

`Filter` 匹配完整目录路径、跳板名或文件名，支持没有源码的目录；筛选保留祖先，不带入无关兄弟。预览必须显式给 `.pdf` 输出路径，不得覆盖正式分册。`SourceOnly` 只生成 `.typ`、可读代码与清单，不要求 Typst，也不运行打印代码/PDF 验收。

完整构建可加 `-SoloMin 90`，让达到 90 行的条目从奇数页起排；默认 0 只保证独立起页，不为奇偶补空页。

打印设置：**A4 竖向、100% 实际大小、每张纸 1 页 PDF、双面长边翻转**。先试印一页确认字号与边缘，再打印需要的册子。

完整构建在隔离目录完成全部编译和检查后发布。当前实际纸面代码的完整行为测试覆盖 KMP、Manacher、ZFunction/E 和 utils；所有 C++ 条目核对可读代码、排版输入、行数与指纹。详细边界、源码选择协议及后续模板接入见 [Booklet 系统说明](architecture.md)。
