# LinuxHUD .gitignore 使用指南

## 概述

`.gitignore` 文件用于指定 Git 应该忽略的文件和目录。这有助于保持代码仓库的整洁，避免提交不必要的文件。

## 忽略的文件类型

### 构建文件
```
build/          # 构建目录
dist/           # 发布包目录
package/        # 安装包目录
*.o             # 目标文件
*.a             # 静态库
*.so            # 共享库
```

### 可执行文件
```
linuxhud        # 主程序
test_drm        # 测试程序
test_*          # 其他测试程序
```

### 调试文件
```
*.dSYM/         # macOS 调试符号
core            # 核心转储
core.*
vgcore.*        # Valgrind 核心转储
massif.out.*    # Valgrind 内存分析
callgrind.out.* # Valgrind 性能分析
```

### IDE 和编辑器文件
```
.vscode/        # VS Code
.idea/          # JetBrains IDE
*.sublime-*     # Sublime Text
*~              # Vim 临时文件
*.swp           # Vim 交换文件
tags            # Ctags
```

### 系统文件
```
.DS_Store       # macOS
Thumbs.db       # Windows
.directory      # Linux
```

### 日志和临时文件
```
*.log           # 日志文件
*.tmp           # 临时文件
*.bak           # 备份文件
*.orig          # 原始文件
```

### 测试和覆盖率
```
*.gcda          # GCC 覆盖率数据
*.gcno          # GCC 覆盖率注释
*.gcov          # Gcov 输出
coverage/       # 覆盖率报告
```

### 打包文件
```
*.tar.gz        # 压缩包
*.zip           # ZIP 压缩包
*.deb           # Debian 包
*.rpm           # RPM 包
```

### 依赖目录
```
node_modules/   # Node.js 依赖
vendor/         # PHP 依赖
third_party/    # 第三方库
```

### 缓存文件
```
.cache/         # 缓存目录
__pycache__/    # Python 缓存
*.pyc           # Python 编译文件
```

### 环境文件
```
.env            # 环境变量
.env.local      # 本地环境变量
.venv/          # Python 虚拟环境
```

## 允许的文件

以下文件类型被明确允许（不被忽略）：

### 配置文件
```
!linuxhud.conf      # 项目配置文件
!linuxhud.service   # systemd 服务文件
```

### 文档文件
```
!*.md               # Markdown 文档
!docs/              # 文档目录
```

### 源代码
```
!src/               # 源代码目录
!include/           # 头文件目录
!tests/             # 测试目录
```

### 构建脚本
```
!Makefile           # Make 构建脚本
!meson.build        # Meson 构建文件
!CMakeLists.txt     # CMake 构建文件
```

### 演示和测试脚本
```
!run_demo.sh        # 演示脚本
!test_project.sh    # 项目测试脚本
!test_makefile.sh   # Makefile 测试脚本
```

## 使用场景

### 1. 新克隆仓库后

```bash
# 克隆仓库
git clone <repository-url>
cd linuxhud

# 检查 .gitignore 是否生效
git status

# 应该看不到 build/、*.o 等文件
```

### 2. 构建项目后

```bash
# 构建项目
make

# 检查状态
git status

# build/ 目录不应该出现在未跟踪文件中
```

### 3. 添加新文件时

如果需要添加新的忽略规则：

```bash
# 编辑 .gitignore
vim .gitignore

# 添加新的忽略规则
echo "new_ignored_file" >> .gitignore

# 提交更改
git add .gitignore
git commit -m "Update .gitignore"
```

### 4. 强制添加被忽略的文件

如果确实需要添加被忽略的文件：

```bash
# 强制添加
git add -f path/to/ignored/file

# 提交
git commit -m "Add ignored file"
```

## 常见问题

### Q: 如何忽略所有文件除了特定类型？

```gitignore
# 忽略所有文件
*

# 允许特定类型
!*.c
!*.h
!Makefile
!*.md
```

### Q: 如何忽略特定目录中的文件？

```gitignore
# 忽略 build 目录中的所有文件
build/*

# 但允许 build 目录存在
!build/.gitkeep
```

### Q: 如何忽略所有但保留子目录中的文件？

```gitignore
# 忽略根目录中的所有文件
/*

# 允许特定目录
!src/
!include/
!tests/
```

### Q: 如何查看哪些文件被忽略？

```bash
# 查看被忽略的文件
git status --ignored

# 查看特定文件是否被忽略
git check-ignore -v filename
```

### Q: 如何从仓库中删除已跟踪的文件？

```bash
# 从 Git 中删除（保留本地文件）
git rm --cached filename

# 提交更改
git commit -m "Remove file from tracking"
```

## 最佳实践

1. **提前规划**：在项目开始时就设置好 .gitignore
2. **定期检查**：使用 `git status` 检查是否有意外文件
3. **保持更新**：随着项目发展更新 .gitignore
4. **文档说明**：在 README 中说明 .gitignore 的用途
5. **团队协作**：确保团队成员了解 .gitignore 规则

## 示例工作流

### 1. 初始化项目

```bash
# 创建项目
mkdir linuxhud
cd linuxhud

# 初始化 Git
git init

# 创建 .gitignore
cat > .gitignore << 'EOF'
# 构建文件
build/
*.o
*.a
linuxhud
test_drm
EOF

# 添加文件
git add .
git commit -m "Initial commit"
```

### 2. 开发过程中

```bash
# 编辑代码
vim src/main.c

# 构建项目
make

# 检查状态（build/ 不应出现）
git status

# 提交更改
git add src/main.c
git commit -m "Update main.c"
```

### 3. 发布版本

```bash
# 清理构建
make clean

# 创建发布包
make dist

# 检查状态（dist/ 不应出现）
git status

# 标记版本
git tag -a v0.1.0 -m "Version 0.1.0"
git push origin v0.1.0
```

## 调试 .gitignore

### 检查文件是否被忽略

```bash
# 检查特定文件
git check-ignore -v build/bin/linuxhud

# 输出示例
# .gitignore:3:build/	build/bin/linuxhud
```

### 查看所有忽略规则

```bash
# 查看当前忽略规则
git ls-files -i --exclude-standard

# 查看详细信息
git ls-files -v | grep ^h
```

### 测试忽略规则

```bash
# 创建测试文件
touch test_ignore.o

# 检查是否被忽略
git status

# 清理
rm test_ignore.o
```

## 相关资源

- [Git 官方文档 - .gitignore](https://git-scm.com/docs/gitignore)
- [GitHub - A collection of .gitignore templates](https://github.com/github/gitignore)
- [gitignore.io - 生成 .gitignore 文件](https://www.toptal.com/developers/gitignore)

---

更多信息请参考 Git 官方文档。
