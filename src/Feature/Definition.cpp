#include "Server/Server.h"
#include "Server/Convert.h"
#include "Compiler/Compilation.h"
#include "Protocol/Feature/Definition.h"

namespace clice {

auto Server::on_definition(proto::DefinitionParams params) -> Result {
    auto path = mapping.to_path(params.textDocument.uri);
    auto opening_file = opening_files.get_or_add(path);
    auto version = opening_file->version;

    auto guard = co_await opening_file->ast_built_lock.try_lock();
    auto ast = opening_file->ast;

    if(opening_file->version != version || !ast) {
        co_return json::Value(nullptr);
    }

    auto offset = to_offset(kind, opening_file->content, params.position);

    co_return co_await async::submit([kind = this->kind, offset, &ast, mapping = this->mapping] {
        // 这里需要实现具体的定义查找逻辑
        // 目前先返回空数组，等待后续实现
        std::vector<proto::Location> locations;
        
        // TODO: 实现基于 AST 的定义查找
        // 1. 在 AST 中查找光标位置的符号
        // 2. 查找该符号的定义位置
        // 3. 转换为 LSP Location 格式
        
        return json::serialize(locations);
    });
}
}  // namespace clice