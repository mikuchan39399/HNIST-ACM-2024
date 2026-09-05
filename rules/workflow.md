# 工具与刷题流程

本文件承接 rule.md 第 10、13 节, 与主规则同级; 修改相关工具时必读。

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

## 13. 刷题工作流 (zoi 全套, 已建成并全链路验证)
日常循环: cph 建题+爬样例 → 写短名 include(补全) → cph 测样例 → Ctrl+S
  → Ctrl+Shift+B 原地展开+复制剪贴板 → 提交 → WA 则命令面板 zoi-restore
  回紧凑态改 → 收工批量回溯。
换机部署: scripts\install-zoi.ps1 一条命令(用户级 settings 写 includePath
  + cph -I, 顺带跑 make_stubs 存状态); uninstall 按状态可逆撤除。
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

scripts\zoi.ps1: expand <file> 备份 A.zoi.cpp 并把递归展开(内联本地
  include + 钻石去重 + 保留守卫)原地写回, 顺带复制剪贴板; restore <dir>
  批量回溯; status 干跑。
接口: Ctrl+Shift+B = zoi-expand; 命令面板 zoi-restore / zoi-booklet(
  重建手册 PDF, 任务在用户全局 tasks.json)。cph 带 -I<库根>\zoi(user
  settings), 直编与展开态共用短 include; IntelliSense
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

