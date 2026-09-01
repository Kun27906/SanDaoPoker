# third_party — 第三方库

存放项目依赖的第三方库。

## 当前依赖

- **SFML 2.6.1**（graphics / window / system / audio / network）
  - `SFML-2.6.1.zip` — 官方预编译包（**随仓库分发**，克隆即得）
  - CMake 首次构建自动解压到 `SFML-2.6.1/`（该目录已被 `.gitignore` 排除，不入库）
  - 解压后 DLL 自动拷贝到 exe 旁（见根 `CMakeLists.txt`）

## 更新方式

替换 SFML 版本时：删除旧 zip，放入新版 zip（如 `SFML-2.7.0.zip`），
并同步修改根 `CMakeLists.txt` 中的 `SFML_ROOT` 与 `.gitignore` 中的解压目录名。
