# 打印比赛手册

先准备 Typst：放入 PATH，或把 typst.exe 放在 scripts 目录。然后在库根目录运行：

```powershell
./scripts/make_booklet.ps1
```

输出根目录 zoi-booklet.pdf。默认 A4 横排三栏，带目录、页码和代码指纹；catalog 的顺序决定条目顺序，笔记会一并收录。

只看某一部分，或为双面打印安排较长条目从正面开始：

```powershell
./scripts/make_booklet.ps1 -Filter seg
./scripts/make_booklet.ps1 -SoloMin 90
```

筛选版不执行全库覆盖审计；赛前生成不带 Filter 的完整版本，并检查纸面排版。PDF 和 booklet.typ 为生成物，不提交到 Git。

实现约定见 [维护说明](../maintenance/README.md)。
