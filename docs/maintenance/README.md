# 工具维护说明

这里只放修改脚本时需要遵守的实现约定。使用操作从 [文档目录](../README.md) 进入。
以下命令在仓库根目录执行。面向读者的表头、状态、覆盖说明和验证结论统一中文；源码路径、接口名和命令保留原名。生成器也必须输出中文，不能只手改生成结果。

| 改了什么 | 要做什么 |
|---|---|
| 母版登记、跳板名称 | 更新 zoi/_catalog.txt，运行 scripts/make_stubs.ps1；遵守模板契约 |
| catalog 或测试 include 关系 | 运行 scripts/make_reliability.ps1，再运行 scripts/make_features.ps1 |
| 模板实现或传递依赖 | 重跑相关套件; 暂不跑时用 scripts/make_verification.ps1 刷新待重验状态 |
| 测试行为、API 或边界用例 | 核对 rules/verification.json 的对应范围, 运行相关套件后查看自动生成的两层表 |
| 指纹、证据或两层表生成器 | 运行 scripts/check_verification_test.ps1, 同时验证 PS5.1 / PS7 |
| 展开/恢复 | 运行 scripts/zoi_check.ps1 |
| 安装/卸载、工作区配置、打包、清理 | 运行 scripts/check_setup.ps1 与 scripts/check_deployment.ps1, PS5.1/PS7 各验, 使用隔离配置 |
| 资产扫描/测试入口 | 运行 scripts/check_inventory_test.ps1 / scripts/check_runner.ps1 |
| 分享队友包 | VS Code 的 zoi-package 或 scripts/make_team_package.ps1, 默认 docs/releases/时间戳 ZIP |

功能细目由 make_features 复用 check_inventory 生成；-Check 只检查且过期失败。每次功能新增、修改或撤下均执行 [rule 双向同步要求](../../rule.md#适用范围与阅读顺序), 核对 LLM 规则与 docs/features/README.md 的用户说明, 不限于新增整个功能; 不手填生成数量或测试通过等级。
所有脚本放 scripts，库根从 PSScriptRoot 推导，不写本机绝对路径。打包收源码、受管文档、验证范围 JSON、自动运行证据及压力入口依赖的 .github 配置, 排除恢复备份、原始测试日志和已有发布包。

安装 v3 管理 settings/tasks/keybindings 三个文档, v2 原快照可升级, 重装检查并补缺项;
卸载精确恢复原文或保留后续无关修改。未知旧状态仍保留, 不猜测归属。AdoptExistingTasks 是显式的手写任务迁移,
只接管指向当前库相应脚本的已知任务, 保留原快照供卸载恢复, 不自动接管冲突命令。
启用补全/错误提示时记录旧值; 不安装扩展, 不改编译器 PATH。项目显式 includePath 可能覆盖用户默认值,
configure-zoi 只处理调用方指定的工作区, 多个 C/C++ 配置逐一补路径; 手动改过的局部配置不强制回滚。
快捷键冲突保留, 用 Ctrl+Alt+T / Ctrl+Alt+Z 提供固定入口, 不篡改 VS Code 的命令历史或抢占 Ctrl+Shift+P。
打包排除规则必须相对库根计算, 不能因为库本身位于 .zoi-checks 或 releases 下就排空全部源码;
拒绝跟随链接, 排除个人状态, 完成 ZIP 后再公布正式文件, 清理临时目录有有限重试。
部署自检必须实际解压发布包并安装, 不能只检查 ZIP 名称。CI setup 作业自动跑两版 PowerShell 的部署自检。

## CI 与报告

push、PR 或手动触发 CI。普通回归、语法扫描、sanitizer、独立压力与 Windows 安装脚本检查分工见 [测试说明](../../scripts/checks.md) 和 [CI 配置](../../.github/workflows/ci.yml)。新增验收用的压力参数须绑定自动作业, 不以保留手动脚本代替接入; 改入口时同时验证成功、失败和超时路径。
报告绑定当次源码、环境、参数和范围；本地绿和已发布提交的绿不能代替未提交改动的验证。日志无权限时不猜原因，尽量本地复现。没有运行的环境不标通过。

两层表的数据分工、失效条件、CI 附件导入与 AI 收尾流程统一见 [验证指南](../verification/README.md), 不在这里复制状态规则. 新增功能时同步用户功能总览和 AI 路由, 不只改实施记录

## 跳板生成约定

zoi\ 跳板层: stub = 一行 #include 指向真身, 纯 ASCII 短名, 中文路径只在
  zoi\_catalog.txt 出现; 新引擎入库 = catalog 加一行 + 跑 make_stubs。
  后缀必须 .h(.cpp 不进补全候选); 命名标准缩写留任、冷门驼峰全称;
  make_stubs 顺手盖 '// zoi: 名' 戳(预检校验一致); ! 豁免行 = 不配跳板
  的决策留痕, 预检做全覆盖校验; 自清洁删改名残留, 纯大小写改名 NTFS
  覆盖写不换名, 要先手删旧件。


## 展开器与事务约定

scripts\zoi.ps1: expand <file.cpp> 原地展开并复制剪贴板; 再次 expand
  先把生成块折回 include, 保留块外修改, 再按当前库重新解析依赖。
  根文件每条本地 include 对应一对 // zoi:begin/end 标记; 钻石依赖按
  母版完整路径去重, 纯跳板只负责定位; 无守卫文件仍按引用次数展开。
  标准 include guard 保留, #pragma once 转为展开器去重、不写入题目正文。
  状态只放 A.zoi.state.json, 请与 A.cpp 一起保留到恢复或解除管理。
接口: 安装器仅在原先没有默认构建任务时将 zoi-expand 设为默认；任务面板可直接选择 zoi-expand / zoi-restore。手动配置的 zoi-booklet(
  重建手册 PDF, 任务在用户全局 tasks.json)。cph 带 -I<库根>\zoi(user
  settings), 直编与展开态共用短 include; IntelliSense 还需启用 Microsoft C/C++ 并确认当前项目配置,
  用户默认 includePath 不能证明实际生效, 工作区设置/编译数据库优先时按安装指南诊断。
操作约定: 编辑器先 Ctrl+S; 新增 include 和题解修改放在生成块外。
  restore <file.cpp|dir> 只折回生成块, 保留 solve 等块外修改; 批量时
  每题独立处理。status <file.cpp|dir> 只读报告可恢复、冲突或孤儿状态。
  删除完整生成块表示删掉该 include; 再次 expand 会重算共享依赖。
  生成块内部被改时普通 expand/restore 报出块号并保留现场:
  可以撤销该块编辑, 或 forget <file.cpp> 保留现有代码并移除管理标记
  和状态, 此后作为自包含源码使用。forget 不回写模板库, 不负责修复
  被用户改坏的 C++。不要只删两条边界而留下正文, 工具无法分辨其来源。
  restore -Force 显式丢弃修改, 回到最近一次展开保存的紧凑快照。
  -NoClipboard 可关闭复制, 对拍使用此参数。
支持边界: 普通无条件本地 include、标准 #ifndef/#define/#endif 守卫、
  文件开头的 #pragma once。条件分支内的本地 include、宏式 include、
  #undef、反斜杠续行、被块注释拆开的本地 include 和无法解析的本地
  路径在写入前报错; 库内已知短名 <name.h> 从 zoi 解析, 不被源目录同名文件遮蔽;
  其他标准库 <...> include 原样保留。不模拟完整预处理器,
  符号链接别名暂不承诺去重。
恢复与清理: 成功 restore/forget 删除本题状态; 写入中断会留下
  A.zoi.pending.json, 下次修改命令先完成事务。如果中断后源码又改过,
  则保留源码与事务记录, 不猜测覆盖。并发命令由本题临时锁隔离。
  旧 A.zoi.cpp + A.zoi.sha 只有指纹吻合才自动迁移; 修改过或缺少 SHA
  的旧备份保留并报错, 不按时间戳冒险恢复。缺源文件也不删唯一备份。
  未知临时文件只报告, 不扫删目录。展开器不创建存档文件夹;
  .zoi-checks 是测试日志目录；新版安装器不创建 .bak，安装事务状态为 settings.json.zoi-state。旧备份不能当成新版残留猜删。
动机: 展开态原地写入 = 被测与提交同一份且零依赖库路径, 状态快照保留
  紧凑视角。坑史(mtime→SHA1 判据演进)详版见 history。

旧版冲突清理：forget 在旧 .zoi.cpp 与合法 SHA 配对存在时保留当前源码，只删除管理文件；缺 SHA 或源码保留备份。禁止用强制回滚代替清理。

## 手册生成约定

封面显示生成日期、筛选范围与使用导览; quick 卡片等高, entrymeta 统一代码信息条, 页眉方向用浅灰标签。代码仍按原字号自动分栏分页。插件附录按相对路径以 zh-CN 显式排序, 避免文件枚举顺序影响页码。视觉改动需对比打印代码内容并渲染抽查封面、目录和正文; 使用说明见 [打印手册](../booklet/README.md)。

赛场纸质化: make_booklet.ps1 -> docs/booklet/output/zoi-booklet-print.pdf(typst A4 横排三栏, 目录带页码);
正式手册只保留这一份 PDF 和同名 .typ, 默认重建覆盖更新, 第 39 页保留 MIKU ♡; 不再维护普通版副本。筛选/排版试验用 OutFile 写入 .zoi-checks/codex-work。修改页脚时整本检查 MIKU 仅在第 39 页, 避免目录页码求值泄漏。
  catalog 顺序即章节序, 行首 ^ = 笔记条目(.txt 正文, 无代码无跳板); 相对
  include 改写为跳板短名(誊写产物=同目录 .h 集合, utils 只印一次); 插件
  附录自动收(含 main 跳过); 每条目印 行数+ SHA256 前 8 hex(LF 归一化),
  自检先数行再对 hash; 纯 =/- 装饰线超栏宽转换期截断(先截后 hash, 纸
  面与指纹一致); 目录两级=域/条目,
  子域是分隔条(subsep), 未代表组的 README 印作导语(subintro); 全库知
  识点文件夹构建期对账（数量以本次扫描为准）, 缺条 exit 1 并打印缺席清单(防缺斤少
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

## 库减负：生成总览与共用资产映射

- 采纳 reliability 自动生成，但它只展示当前测试资产关系，不从 include 推断独立暴力、API 覆盖、运行通过或赛场可靠性。A/B/C 若保留，只是导航：直接引用资产/未发现直接引用/纯文本笔记；豁免 C++ 单列，不混入笔记。
- 独立入口 scripts/make_reliability.ps1，默认生成，-Check 只比较且过期失败；不在普通 run_checks 中写跟踪文档。CI 普通作业校验生成物，无时间戳、绝对路径或不稳定排序。
- scripts/check_inventory.ps1 共用 catalog/源文件/直接 include 事实映射，runner 和生成器都调用，不维护第二份模板到套件表、不解析 TEST GAP 控制台文本。抽取时保留现有 scaffold 校验失败语义，并更新 check_runner 的假仓库依赖。
- TEST GAP 继续是提示，不作门禁，不把传递 include 转成行为覆盖率。旧无指纹 summary 不导入当前证据; 现行 runner 已记录源码及依赖指纹, 失效判断统一见验证指南。
- 旧 reliability 全表已完整归档至 sweep-history；深检与重构期标记留在历史，不作为源码变化后自动继承的等级。rule 的手工同步义务已替换为生成和校验，测试纪律不变。
- 修改资产映射时验证映射增删、同名不同路径、笔记/豁免、输出幂等、-Check 不写且过期失败和历史保留; 运行现行入口自检, 不手填会过期的测试项数, 不重扫无关模板。
- 六个规则分卷默认不增文件，新内容放现有职责内；不为凑数重新合成巨型 rule。
- 历史压缩保留问题结论、测试环境/范围/结果、证据位置和未解决分歧；去掉重复进度即可，不自动改写为更强结论。暂不做历史压缩脚本。
日常命令: ./scripts/make_reliability.ps1 更新总览; -Check 只检查是否过期。
修改算法正文但不改变资产或 include 关系时无须重生成；它不登记算法运行结果。
映射自检: ./scripts/check_inventory_test.ps1；检查入口自检: ./scripts/check_runner.ps1。

模板和知识点变更收尾执行 [学习与入库进度同步约定](../progress/README.md#ai-何时询问如何同步)，核对条目、链接、摘要和用户确认；不由生成器覆盖学习记录。

纸质化回归须检查：筛选条目的 include 仍按完整 catalog 和相对目标路径改写；同名源文件不按 basename 混淆；SoloMin 的正文和插件均按阈值从奇数页开始，审计不得漏掉应检查锚点。版式修改后核对代码块、目录覆盖与页数，并渲染检查目录、正文、笔记和附录。

工作目录、子进程临时文件与长期交接统一执行 [工作区与沉淀](../../rules/collab.md#工作区与沉淀)。

## 测试缓存保留

`.zoi-checks` 是隔离测试的编译产物、临时输入、配置副本和诊断现场, 不参与模板 include 或安装后的正常刷题。
run_checks/check_setup/zoi_check/check_runner/check_deployment 完成后写 `.zoi-run.json`,
只有 PASS、路径吻合、类型已知的完成目录才可自动清理, 每类保留最近三份。
同一缓存根用临时独占锁串行清理, 同时完成的另一测试跳过本次清理, 避免争删同一现场。
异常退出无完成标记、FAIL、含链接的目录与 codex-work 均保留, 不靠目录名猜成功。

```powershell
./scripts/clean_checks.ps1              # 只预览
./scripts/clean_checks.ps1 -Apply       # 执行清理, 与 zoi-clean-checks 相同
```

长期证据先写 records/verification 或 records/tooling; 两层验证表不依赖本地原始日志存活。
旧版无标记现场由维护者审计后处理, 清理器不批量认领。`.ci-results`、正式 ZIP、私人备份不自动删除。
跨 Windows/WSL 的绝对路径不同, 各环境只清理路径与自身一致的现场; 不把异平台路径猜映射后删除。
人工工作区里的下载包、工具和复现脚本需确认用途及证据沉淀后按项清理, 不能把整个目录直接删掉。

根目录只保留 README.md、AGENTS.md、rule.md 和 .gitignore 等必要入口。旧规则历史位于 records/tooling/rule_history.md；手册 PDF 及同名 .typ 默认生成到 docs/booklet/output，指定 OutFile 时两者跟随该路径，不能重新把默认生成物散落到根目录。打包排除手册 output，仍保留历史正文。
