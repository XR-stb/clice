# Running clice with Docker

This guide explains how to build and run clice using Docker containers.

## Prerequisites

- Docker installed on your system
- Your C++ project source code

## Option 1: Using Pre-built Release (Recommended)

If you don't want to compile from source, you can use the official pre-built release:

```bash
# Create a simple Dockerfile using the pre-built version
cat > Dockerfile.prebuilt << 'EOF'
FROM ubuntu:24.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    wget \
    xz-utils \
    && rm -rf /var/lib/apt/lists/*

# Download and install pre-built clice
RUN wget -O /tmp/clice.tar.xz https://github.com/clice-io/clice/releases/download/v0.1.0-alpha.1/clice-x86_64-linux-gnu.tar.xz \
    && cd /opt \
    && tar -xf /tmp/clice.tar.xz \
    && rm /tmp/clice.tar.xz \
    && ln -s /opt/clice-x86_64-linux-gnu/bin/clice /usr/local/bin/clice

WORKDIR /workspace
ENTRYPOINT ["clice"]
EOF

# Build the image
docker build -f Dockerfile.prebuilt -t clice:prebuilt .
```

## Option 2: Building from Source

The project includes a multi-stage Dockerfile that builds clice from source. To build the Docker image:

```bash
# Clone the repository (if you haven't already)
git clone https://github.com/clice-io/clice.git
cd clice

# Build the Docker image
docker build -t clice:latest .
```

The build process will:
1. Install system dependencies (ninja-build, cmake, build-essential, etc.)
2. Install LLVM 20
3. Build clice from source
4. Create a minimal runtime image with the clice binary

## Running clice in Docker

### Interactive Mode

To run clice interactively in a Docker container:

```bash
# Using pre-built version
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  clice:prebuilt --help

# Or using source-built version
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  clice:latest --help
```

### Language Server Mode

For use as a language server, you'll typically want to run clice in pipe mode:

```bash
# Using pre-built version (recommended)
docker run -i --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:prebuilt --mode=pipe

# Or using source-built version
docker run -i --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:latest --mode=pipe
```

### Socket Mode (for debugging)

To run clice in socket mode for debugging purposes:

```bash
# Run clice in socket mode
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  -p 9999:9999 \
  clice:latest --mode=socket
```

## Configuration

### Using a Custom Configuration File

If you have a custom `clice.toml` configuration file:

```bash
# Mount your config file
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -v /path/to/your/clice.toml:/workspace/clice.toml \
  -w /workspace \
  clice:latest --config=/workspace/clice.toml
```

### Environment Variables

You can set environment variables for the container:

```bash
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  -e CLICE_LOG_LEVEL=debug \
  clice:latest
```

## Complete Example

Here's a complete example of running clice with a C++ project:

```bash
# Assuming your project structure is:
# /home/user/my-cpp-project/
# ├── src/
# ├── include/
# ├── build/
# │   └── compile_commands.json
# └── clice.toml

# Run clice as a language server
docker run -i --rm \
  -v /home/user/my-cpp-project:/workspace \
  -w /workspace \
  clice:latest \
  --config=/workspace/clice.toml \
  --mode=pipe
```

## Docker Compose

For more complex setups, you can use Docker Compose. Create a `docker-compose.yml` file:

```yaml
version: '3.8'

services:
  clice:
    build: .
    volumes:
      - ./:/workspace
    working_dir: /workspace
    stdin_open: true
    tty: true
    command: ["--mode=pipe"]
```

Then run:

```bash
docker-compose up clice
```

## Troubleshooting

### Resource Directory Issues

If you encounter warnings about missing resource directories:

```bash
# Specify the resource directory explicitly
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  clice:latest \
  --resource-dir=/opt/clice/lib/clang/20
```

### Permission Issues

If you encounter permission issues with mounted volumes:

```bash
# Run with your user ID
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  --user $(id -u):$(id -g) \
  clice:latest
```

### Debugging

To debug issues, you can run the container interactively:

```bash
# Get a shell in the container
docker run -it --rm \
  -v /path/to/your/project:/workspace \
  -w /workspace \
  --entrypoint /bin/bash \
  clice:latest

# Then run clice manually inside the container
root@container:/workspace# clice --help
```

## Integration with IDEs

### VS Code with Dev Containers

Create a `.devcontainer/devcontainer.json` file in your project:

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

This allows you to develop inside the container with full clice support.