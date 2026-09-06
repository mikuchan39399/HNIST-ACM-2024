# 队友安装包

本目录是正式的本地发布输出目录, ZIP 不提交 Git。解压后按包内 docs/setup/README.md 安装。

在 VS Code 运行 zoi-package, 或在仓库根目录运行:

    ./scripts/make_team_package.ps1

默认文件名为 HNIST-ZOI-yyyyMMdd-HHmmss-fff.zip, 终端显示完整路径; 可用 -OutputPath 指定其他文件。
已有同名 ZIP 不覆盖, 只有压缩完成才改成正式文件名。旧 ZIP 按分享需要手动保留或删除, 不被缓存清理器删除。
包收当前源码(含未提交修改), 不含 Git 历史、个人安装记录、编辑器配置、展开状态、编译产物、
原始日志、旧发布包或私人备份; 正式 PDF 另行发送。打包不执行全库对拍, 发布检查由维护者负责。
releases/backups 只收 README 指南, 避免解压后文档导航断链。
