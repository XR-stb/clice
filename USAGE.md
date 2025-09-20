# clice 使用指南

## 概述

clice 是一个现代化的 C++ 语言服务器，提供代码补全、语法检查、符号导航等功能。本文档将指导你正确安装和配置 clice。

## 安装步骤

### 1. 获取 clice 二进制文件

从 [releases 页面](https://github.com/clice-io/clice/releases) 下载最新的 clice 二进制文件，或者从源码编译。

### 2. 安装 VS Code 扩展

在 VS Code 中安装 [clice 扩展](https://marketplace.visualstudio.com/items?itemName=ykiko.clice-vscode)。

### 3. 准备资源目录

clice 需要 clang 资源目录来正常工作。你有两个选择：

#### 选项 A：使用系统 clang 资源目录（推荐）
```bash
# 查找系统 clang 资源目录
clang --print-resource-dir
# 输出示例：/usr/lib/clang/18
```

#### 选项 B：创建本地资源目录
```bash
# 在项目目录下创建资源目录结构
mkdir -p lib/clang/20/include
# 从系统复制必要的头文件
cp -r /usr/lib/clang/*/include/* lib/clang/20/include/
```

## VS Code 配置

在项目的 `.vscode/settings.json` 中添加以下配置：

### Socket 模式配置（推荐用于开发调试）
```json
{
    "clangd.path": "",
    "clice.path": "/path/to/your/clice",
    "clice.mode": "socket",
    "clice.host": "127.0.0.1",
    "clice.port": 50051
}
```

### Pipe 模式配置（推荐用于生产环境）
```json
{
    "clangd.path": "",
    "clice.path": "/path/to/your/clice",
    "clice.mode": "pipe",
    "clice.arguments": [
        "--config=clice.toml",
        "--resource-dir=/usr/lib/clang/18"
    ]
}
```

### 使用相对路径的配置示例
```json
{
    "clangd.path": "",
    "clice.path": "./clice",
    "clice.mode": "pipe",
    "clice.arguments": [
        "--config=clice.toml",
        "--resource-dir=./lib/clang/20"
    ]
}
```

**重要提示：**
- 使用 `clice.path` 而不是 `clice.executable`
- 路径必须指向可执行文件，不是目录
- 例如：`/home/user/clice/clice` 而不是 `/home/user/clice/`

## 项目配置

### 1. 创建 clice.toml 配置文件

在项目根目录创建 `clice.toml` 文件：

```toml
[project]
    # 启用实验性 clang-tidy 诊断
    clang_tidy = false
    
    # 内存中保持的最大活跃文件数
    max_active_file = 8
    
    # 缓存目录
    cache_dir = "${workspace}/.clice/cache"
    
    # 索引目录
    index_dir = "${workspace}/.clice/index"
    
    # 日志目录
    logging_dir = "${workspace}/.clice/logging"
    
    # 编译命令搜索目录
    compile_commands_dirs = ["${workspace}/build"]

[[rules]]
    patterns = ["**/*"]
    append = []
    remove = []
```

### 2. 生成编译数据库

clice 需要 `compile_commands.json` 来理解项目结构：

#### 使用 CMake
```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build
```

#### 使用 Bear（适用于 Makefile 项目）
```bash
bear -- make
```

## 启动方式

### Socket 模式（推荐）

1. 手动启动 clice 服务器：
```bash
./clice --config=clice.toml --resource-dir=/usr/lib/clang/18 --mode=socket
```

2. 启动 VS Code，扩展会自动连接到服务器

### Pipe 模式

VS Code 扩展会自动启动 clice 进程，无需手动启动服务器：

```json
{
    "clice.path": "/path/to/your/clice",
    "clice.mode": "pipe",
    "clice.arguments": [
        "--config=clice.toml",
        "--resource-dir=/usr/lib/clang/18"
    ]
}
```

**特点：**
- VS Code 自动管理进程生命周期
- 通过 stdin/stdout 通信
- 性能更好，配置更简单
- 适合生产环境使用

## 故障排除

### 1. 权限错误 (EACCES)
```bash
# 确保 clice 可执行文件有执行权限
chmod +x /path/to/clice
```

**常见原因：**
- 路径指向目录而不是可执行文件
- 配置参数名错误（应使用 `clice.path` 而不是 `clice.executable`）
- 文件没有执行权限

**解决方案：**
```bash
# 检查文件类型
file /path/to/clice
# 应该显示：ELF 64-bit LSB executable

# 检查权限
ls -la /path/to/clice
# 应该有 x 权限：-rwxr-xr-x

# 正确的配置示例
{
    "clice.path": "/home/user/clice/clice",  // 指向可执行文件
    "clice.mode": "pipe"
}
```

### 2. 找不到头文件 ('stddef.h' file not found)
- 检查资源目录路径是否正确
- 确保资源目录包含必要的头文件
- 使用绝对路径而非相对路径

### 3. PCH 构建失败
- 检查 `compile_commands.json` 是否存在且正确
- 确保编译命令中的包含路径正确
- 检查项目依赖是否完整

### 4. 连接失败
- Socket 模式：确保端口未被占用，手动启动服务器
- Pipe 模式：检查可执行文件路径和权限

### 5. 代码补全不工作
- 确保有有效的 `compile_commands.json`
- 检查日志中的 PCH 构建状态
- 验证资源目录配置

## 日志调试

启用详细日志：
```bash
./clice --config=clice.toml --resource-dir=/usr/lib/clang/18 --mode=socket --log-level=debug
```

查看日志文件：
```bash
# 日志通常保存在项目的 .clice/logging 目录
tail -f .clice/logging/clice.log
```

## 性能优化

### 1. 调整缓存设置
```toml
[project]
    max_active_file = 16  # 增加活跃文件数
    cache_dir = "/tmp/clice_cache"  # 使用更快的存储
```

### 2. 排除不必要的文件
```toml
[[rules]]
    patterns = ["**/build/**", "**/.*/**"]
    # 排除构建目录和隐藏目录
```

## 功能特性

### 当前支持的功能
- 代码补全
- 语法高亮和错误检查
- 悬停提示
- 代码折叠
- 文档符号
- 内联提示

### 开发中的功能
- 跳转到定义
- 查找引用
- 重命名符号
- C++20 模块支持
- 头文件上下文切换

## 注意事项

1. clice 目前是早期版本，不建议在生产环境使用
2. 某些功能仍在开发中，可能不稳定
3. 建议同时保留 clangd 作为备选方案
4. 定期更新到最新版本以获得最佳体验

## 获取帮助

- [官方文档](https://clice.io)
- [GitHub Issues](https://github.com/clice-io/clice/issues)
- [Discord 社区](https://discord.gg/PA3UxW2VA3)