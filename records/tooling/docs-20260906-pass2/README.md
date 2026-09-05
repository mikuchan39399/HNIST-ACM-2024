# 功能总览与使用/维护分离

2026-09-06。此目录保存本轮改动前的 README、文档导航和 workflow 原文，复制时逐文件校验 SHA256。

- 日常功能入口：docs/features/README.md；完整条目：docs/features/catalog.md，从 catalog 共用资产映射自动生成。
- workflow 常用底座留原位置；跳板、展开事务、手册排版和资产映射约定迁入 docs/maintenance/README.md。
- 安装操作已有 docs/setup/README.md，展开操作已有 docs/expand/README.md；纸质手册操作单列 docs/booklet/README.md。重复的旧换机流程留本快照追溯，不再作为现行步骤。
- 原来的固定数量与历史通过次数不作为持续保证；divCeilFloor 已有跳板，修正旧“暂无跳板”说明。
- 原 README 的个人文字与参考链接保留。所有算法母版与算法契约未改。
- 已验：PS5.1/PS7 目录生成和只读校验、过期检测退出 1 且不写、重复生成不改文件、功能页与指南链接、actionlint 与 diff 格式检查。CI 新增目录过期检查，尚未推送验证线上。