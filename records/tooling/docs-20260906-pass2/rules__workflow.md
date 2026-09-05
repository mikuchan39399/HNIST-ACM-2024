# 工具与刷题流程

本文件记录工具维护约定；修改相关脚本时阅读。日常操作先看 docs/README.md，算法契约见 rule.md。

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

## 13. 刷题工作流
日常循环: cph 建题+爬样例 → 写短名 include(补全) → cph 测样例 → Ctrl+S
  → 运行 zoi-expand 原地展开+复制剪贴板 → 提交 → WA 则命令面板 zoi-restore
  回紧凑态改 → 收工批量回溯。
换机部署: 队友包解压后双击 scripts\install-zoi.cmd；用户级 includePath
  + CPH 参数与五个任务统一注册，先校验自带跳板。双击 uninstall-zoi.cmd 可逆撤配置并清除未改的受管包；详见 [队友安装说明](../docs/setup/README.md)。
scripts\: 全部脚本在此, 根目录只留 rule/规则分卷(rules\)与协作入口
  (AGENTS.md, .clinerules\)/清单/README/.gitignore; 脚本里
  库根一律 Split-Path -Parent $PSScriptRoot。
CI(GitHub Actions): push/PR/手动运行触发 ci.yml, Linux 普通回归+全 cpp 语法扫描,
另跑 ASan/UBSan; 主入口、失败日志和时限见 scripts/checks.md。不再跳过 Linux 快读写。
先打印编译器版本, 报告绑定本次运行; 已发布提交的绿灯不能证明未提交的本地改动已通过。
历史调试经验仍适用: 本地绿是义务, CI 红要定位; 匿名 actions/runs API 可读状态,
私有日志或无权限日志不能猜, 优先用本机 WSL 复现。


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

scripts\zoi.ps1: expand <file.cpp> 原地展开并复制剪贴板; 再次 expand
  先把生成块折回 include, 保留块外修改, 再按当前库重新解析依赖。
  根文件每条本地 include 对应一对 // zoi:begin/end 标记; 钻石依赖按
  母版完整路径去重, 纯跳板只负责定位; 无守卫文件仍按引用次数展开。
  标准 include guard 保留, #pragma once 转为展开器去重、不写入题目正文。
  状态只放 A.zoi.state.json, 请与 A.cpp 一起保留到恢复或解除管理。
接口: 安装器仅在原先没有默认构建任务时将 zoi-expand 设为默认；任务面板可直接选择 zoi-expand / zoi-restore。手动配置的 zoi-booklet(
  重建手册 PDF, 任务在用户全局 tasks.json)。cph 带 -I<库根>\zoi(user
  settings), 直编与展开态共用短 include; IntelliSense
  已接(includePath 含 zoi): 补全/悬停/F12 穿透跳板均可用。
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
  路径在写入前报错; 标准库 <...> include 原样保留。不模拟完整预处理器,
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
迁移 runbook(换机半小时): ① cpptools+cph ② settings 加 includePath +
  cph -I ③ user tasks 指 zoi.ps1 ④ clone ⑤ run_checks 绿 + expand 验证。
  脚本零硬编码路径; cph 只管建题/测面板; PS5.1 防御写法(纯 ASCII、
  cmd /c), PS7 兼容。


## 库减负：生成总览与共用资产映射

- 采纳 reliability 自动生成，但它只展示当前测试资产关系，不从 include 推断独立暴力、API 覆盖、运行通过或赛场可靠性。A/B/C 若保留，只是导航：直接引用资产/未发现直接引用/纯文本笔记；豁免 C++ 单列，不混入笔记。
- 独立入口 scripts/make_reliability.ps1，默认生成，-Check 只比较且过期失败；不在普通 run_checks 中写跟踪文档。CI 普通作业校验生成物，无时间戳、绝对路径或不稳定排序。
- scripts/check_inventory.ps1 共用 catalog/源文件/直接 include 事实映射，runner 和生成器都调用，不维护第二份模板到套件表、不解析 TEST GAP 控制台文本。抽取时保留现有 scaffold 校验失败语义，并更新 check_runner 的假仓库依赖。
- TEST GAP 继续是提示，不作门禁，不把传递 include 转成行为覆盖率。第一版不合并旧 PASS；现有 summary 无源码及依赖指纹，不能证明当前脏工作区通过。运行结果仍查报告中的环境/参数/范围。
- 旧 reliability 全表已完整归档至 sweep-history；深检与重构期标记留在历史，不作为源码变化后自动继承的等级。rule 的手工同步义务已替换为生成和校验，测试纪律不变。
- 验收仅围绕映射增删、同名不同路径、笔记/豁免、输出幂等、-Check 不写且过期失败、旧内容有去处；复跑 runner 九项自检，不重扫无关模板。
- 六个规则分卷默认不增文件，新内容放现有职责内；不为凑数重新合成巨型 rule。
- 历史压缩保留问题结论、测试环境/范围/结果、证据位置和未解决分歧；去掉重复进度即可，不自动改写为更强结论。暂不做历史压缩脚本。
日常命令: ./scripts/make_reliability.ps1 更新总览; -Check 只检查是否过期。
修改算法正文但不改变资产或 include 关系时无须重生成；它不登记算法运行结果。
映射自检: ./scripts/check_inventory_test.ps1；检查入口自检: ./scripts/check_runner.ps1。

旧版冲突清理: forget <file.cpp> 在旧 .zoi.cpp 与合法 SHA 旁置文件存在时，只删除管理文件，当前源码逐字保留（包括旧展开注释）。缺 SHA/源文件仍保留备份，禁止强制回滚代替清理。

安装卸载验证: scripts/check_setup.ps1 只用隔离配置，Windows CI 分别跑 PS5.1 和 PS7。原配置无后续修改时逐字恢复；后续增加的用户配置保留；安装状态和工具临时文件成功后清掉。旧格式/无归属状态不猜删。队友包由 make_team_package.ps1 -OutputPath <zip> 生成，不含个人配置、Git 元数据和测试日志。
