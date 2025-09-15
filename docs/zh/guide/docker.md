# 使用 Docker 运行 clice

本指南介绍如何使用 Docker 容器构建和运行 clice。

## 前置要求

- 系统已安装 Docker
- 你的 C++ 项目源代码

## 方式一：使用预编译版本（推荐）

如果你不想编译源代码，可以直接使用官方发布的预编译版本：

```bash
# 创建一个简单的 Dockerfile 使用预编译版本
cat > Dockerfile.prebuilt << 'EOF'
FROM ubuntu:24.04

# 安装运行时依赖
RUN apt-get update && apt-get install -y \
    wget \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

# 下载并安装预编译的 clice
RUN wget -O /tmp/clice.tar.xz https://github.com/clice-io/clice/releases/download/v0.1.0-alpha.1/clice-x86_64-linux-gnu.tar.xz \
    && cd /opt \
    && tar -xf /tmp/clice.tar.xz \
    && rm /tmp/clice.tar.xz \
    && ln -s /opt/clice-x86_64-linux-gnu/bin/clice /usr/local/bin/clice

WORKDIR /workspace
ENTRYPOINT ["clice"]
EOF

# 构建镜像
docker build -f Dockerfile.prebuilt -t clice:prebuilt .
```

## 方式二：从源代码构建

项目包含一个多阶段 Dockerfile，可以从源代码构建 clice。构建 Docker 镜像：

```bash
# 克隆仓库（如果还没有的话）
git clone https://github.com/clice-io/clice.git
cd clice

# 构建 Docker 镜像
docker build -t clice:latest .
```

构建过程将：
1. 安装系统依赖（ninja-build、cmake、build-essential 等）
2. 安装 LLVM 20
3. 从源代码构建 clice
4. 创建包含 clice 二进制文件的最小运行时镜像

## 在 Docker 中运行 clice

### 交互模式

在 Docker 容器中交互式运行 clice：

```bash
# 使用预编译版本
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  clice:prebuilt --help

# 或使用从源代码构建的版本
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  clice:latest --help
```

### 语言服务器模式

作为语言服务器使用时，通常需要在管道模式下运行 clice：

```bash
# 使用预编译版本（推荐）
docker run -i --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:prebuilt --mode=pipe

# 或使用从源代码构建的版本
docker run -i --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:latest --mode=pipe
```

### Socket 模式（用于调试）

以 socket 模式运行 clice 进行调试：

```bash
# 使用预编译版本
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  -p 9999:9999 \
  clice:prebuilt --mode=socket

# 或使用从源代码构建的版本
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  -p 9999:9999 \
  clice:latest --mode=socket
```

## 配置

### 使用自定义配置文件

如果你有自定义的 `clice.toml` 配置文件：

```bash
# 挂载你的配置文件
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -v /path/to/your/clice.toml:/workspace/clice.toml \
  -w /workspace \
  clice:latest --config=/workspace/clice.toml
```

### 环境变量

你可以为容器设置环境变量：

```bash
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  -e CLICE_LOG_LEVEL=debug \
  clice:latest
```

## 完整示例

以下是使用 C++ 项目运行 clice 的完整示例：

```bash
# 假设你的项目结构是：
# /home/user/my-cpp-project/
# ├── src/
# ├── include/
# ├── build/
# │   └── compile_commands.json
# └── clice.toml

# 使用预编译版本运行 clice（推荐）
docker run -i --rm \
  -v /home/user/my-cpp-project:/workspace \
  -w /workspace \
  clice:prebuilt \
  --config=/workspace/clice.toml \
  --mode=pipe
```

## Docker Compose

对于更复杂的设置，你可以使用 Docker Compose。创建 `docker-compose.yml` 文件：

```yaml
version: '3.8'

services:
  # 使用预编译版本（推荐）
  clice:
    build:
      context: .
      dockerfile: Dockerfile.prebuilt
    volumes:
      - ./:/workspace
    working_dir: /workspace
    stdin_open: true
    tty: true
    command: ["--mode=pipe"]

  # 或使用从源代码构建的版本
  clice-build:
    build: .
    volumes:
      - ./:/workspace
    working_dir: /workspace
    stdin_open: true
    tty: true
    command: ["--mode=pipe"]
```

然后运行：

```bash
# 使用预编译版本
docker-compose up clice

# 或使用从源代码构建的版本
docker-compose up clice-build
```

## 故障排除

### 资源目录问题

如果遇到缺少资源目录的警告：

```bash
# 使用预编译版本时指定资源目录
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:prebuilt \
  --resource-dir=/opt/clice-x86_64-linux-gnu/lib/clang/20

# 使用从源代码构建的版本时指定资源目录
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:latest \
  --resource-dir=/opt/clice/lib/clang/20
```

### 权限问题

如果遇到挂载卷的权限问题：

```bash
# 使用预编译版本时指定用户 ID
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  --user $(id -u):$(id -g) \
  clice:prebuilt

# 使用从源代码构建的版本时指定用户 ID
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  --user $(id -u):$(id -g) \
  clice:latest
```

### 调试

要调试问题，你可以交互式运行容器：

```bash
# 在容器中获取 shell
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  --entrypoint /bin/bash \
  clice:latest

# 然后在容器内手动运行 clice
root@container:/workspace# clice --help
```

## IDE 集成

### VS Code 与 Dev Containers

在你的项目中创建 `.devcontainer/devcontainer.json` 文件：

```json
{
  "name": "C++ with clice",
  "build": {
    "dockerfile": "../Dockerfile"
  },
  "customizations": {
    "vscode": {
      "extensions": ["ykiko.clice-vscode"],
      "settings": {
        "clice.executable": "/opt/clice/bin/clice"
      }
    }
  },
  "mounts": [
    "source=${localWorkspaceFolder},target=/workspace,type=bind"
  ],
  "workspaceFolder": "/workspace"
}
```

这允许你在容器内开发，并获得完整的 clice 支持。