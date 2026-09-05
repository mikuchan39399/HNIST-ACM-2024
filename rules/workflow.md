# 工具规则入口与常用底座

日常操作和实现约定分别维护，本文不再重复它们的正文。

| 需要了解什么 | 现行正文 |
|---|---|
| 库里有什么 | [功能总览](../docs/features/README.md) |
| 安装、卸载、VS Code 任务 | [安装指南](../docs/setup/README.md) |
| 展开、恢复、状态清理 | [刷题指南](../docs/expand/README.md) |
| 打印手册 | [手册指南](../docs/booklet/README.md) |
| 脚本实现、跳板、事务、CI 与目录生成 | [工具维护说明](../docs/maintenance/README.md) |
| 回归与失败定位 | [测试说明](../scripts/checks.md) |

修改工具时必须读对应使用契约和维护约定。迁移前原文保存在 records/tooling/docs-20260906-pass2，历史不作为并行规则。

## 10. 常用底座 (题目代码的地基, 不随题改)
utils 母版(杂项\utils\utils.cpp): 别名全家 + 最值常量 + fast_io +
  debug/debug_array(LOCAL 包裹) + 方向数组(inline, 刻意非 const)。
  自带 using namespace std 和常用头, include 它一个就够。
快读快写(杂项\快读快写\快读快写.cpp, 跳板名 rw): fread/fwrite 手动
  缓冲的 read/write 全家, 析构自动冲刷, utils_int 概念覆盖 __int128
  。库内引擎不用, 刷题按需 include。
散件: custom_hash(杂项\防卡, 跳板 customHash) | floor_div/ceil_div
  (数学\数论, 跳板 divCeilFloor) | i128 别名与流运算符(杂项\128位整数)。
多测入口: main → while(t--) solve(); mid 用 (l+r)>>1 或 l+(r-l)/2。
z_fill_n 坑: 折叠表达式的模式只能是 cast-expression, 二元比较要
  整体加括号 assert(((((int)cs.size()) >= n) && ...));
  旧写法在新版 GCC 直接编译错误。
