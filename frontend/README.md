# AnalyzerClient

一个基于 Qt 和 Clang 的 C++ 代码分析客户端，提供代码质量分析、克隆检测、评分系统等功能。

## 📋 项目简介

AnalyzerClient 是一个功能强大的 C++ 代码分析工具，通过集成 Clang/LLVM 编译器前端，对 C++ 代码进行深度静态分析。该工具提供了直观的图形界面，帮助开发者识别代码问题、检测代码克隆、评估代码质量，并提供详细的代码评分报告。

## 🛠️ 技术栈

### 核心框架
- **Qt 5.15.2** - 跨平台 GUI 应用程序框架
  - Qt Core - 核心功能模块
  - Qt GUI - 图形用户界面
  - Qt Widgets - 窗口部件
  - Qt Network - 网络通信
  - Qt SQL - 数据库支持

### 代码分析引擎
- **Clang/LLVM** - C++ 代码静态分析引擎
  - clangAST - 抽象语法树
  - clangBasic - 基础库
  - clangFrontend - 前端接口
  - clangTooling - 工具支持
  - clangASTMatchers - AST 匹配器

### 第三方库
- **Boost 1.81/1.86** - C++ 扩展库
  - boost::json - JSON 解析和生成
  - boost::container - 容器库
  - boost::asio - 异步 I/O（用于服务器通信）

### 数据库
- **MySQL** - 远程数据库支持
- **SQLite** - 本地数据库（通过 Qt SQL）

### 开发环境
- **C++17** - 编程语言标准
- **CMake** - 构建系统（用于 Clang 工具）
- **qmake** - Qt 项目构建系统
- **MSVC 2019** - Windows 平台编译器

## 🎯 功能模块

### 1. 代码编辑器模块
- **CodeEditor** - 主代码编辑器
  - 语法高亮显示
  - 行号显示
  - 代码标签管理（错误、警告标记）
  - 查找和替换功能
  - 自动完成功能
  - 支持多文件标签页编辑

### 2. 文件管理模块
- **FileManager** - 文件系统管理
  - 项目文件树浏览
  - 文件打开/保存
  - 最近打开项目记录
  - 文件编码支持（UTF-8、GBK）
  - 递归文件搜索

### 3. 代码分析模块
- **AnalyzeThread** - 异步分析线程
  - 后台代码分析
  - 分析进度显示
  - 可中断分析过程
- **AnalyzeResultHandler** - 分析结果处理
  - 错误信息收集
  - 警告信息收集
  - 克隆检测
  - 代码评分计算

### 4. 代码克隆检测模块
- **CloneExtractPreview** - 克隆代码预览
- **CodeComparison** - 代码对比工具
  - 检测重复代码片段
  - 克隆位置标记
  - 克隆集合信息展示

### 5. 代码评分模块
- **ScoreWidget** - 评分展示界面
  - 多维度评分系统：
    - **规范性 (N)** - 代码规范检查
    - **执行效率 (E)** - 性能优化建议
    - **可维护性 (M)** - 代码可维护性评估
    - **安全性 (S)** - 代码风险检测
  - 饼图可视化
  - 评分详情展示

### 6. 代码摘要模块
- **SummaryWidget** - 代码统计摘要
  - 总行数统计
  - 有效行数统计
  - 函数数量统计
  - 变量数量统计
  - 类型定义统计
  - 命名规范统计

### 7. 数据库模块
- **LocalDB** - 本地数据库管理
  - SQLite 数据库操作
  - 本地数据缓存
- **RemoteDB** - 远程数据库管理
  - MySQL 数据库连接
  - 数据同步功能
  - 远程数据推送

### 8. 用户界面模块
- **MainWidget** - 主窗口界面
  - 欢迎页面
  - 分析进度提示
  - 布局管理
- **WelcomePage** - 欢迎页面
- **LoginWidget** - 登录界面
- **SettingWidget** - 设置界面
- **CreateProjectWidget** - 项目创建界面

### 9. 辅助功能模块
- **LabelManager** - 标签管理器（错误/警告标记）
- **LineNumberList** - 行号列表
- **AutoComplete** - 自动完成
- **FindAndReplaceDlg** - 查找替换对话框
- **Toast** - 消息提示
- **PieChart** - 饼图组件

### 10. 服务器通信模块
- **Server** - 本地服务器（基于 Boost.Asio）
  - 消息传输
  - 事件代理
  - 端口监听（默认 12345）

## 📁 项目结构

```
AnalyzerClient/
├── main.cpp                 # 程序入口
├── AnalyzerClient.h/cpp    # 主窗口类
├── MainWidget.h/cpp        # 主界面组件
├── CodeEditor.h/cpp        # 代码编辑器
├── FileManager.h/cpp       # 文件管理器
├── AnalyzeThread.h/cpp     # 分析线程
├── AnalyzeResultHandler.h/cpp  # 分析结果处理
├── ScoreWidget.h/cpp       # 评分组件
├── SummaryWidget.h/cpp     # 摘要组件
├── LocalDB.h/cpp          # 本地数据库
├── RemoteDB.h/cpp         # 远程数据库
├── InfoStruct.h/cpp       # 数据结构定义
├── CMakeLists.txt         # CMake 构建配置
├── AnalyzerClient.pro     # qmake 项目文件
├── images/                # 资源图片
├── lib/                   # 静态库文件
├── plugins/               # 插件目录
└── boost/                 # Boost 库头文件
```

## 🚀 编译与安装

### 前置要求

1. **Qt 5.15.2** 或更高版本
2. **Visual Studio 2019** 或更高版本（Windows）
3. **Clang/LLVM** 开发库
4. **Boost 1.81+** 库
5. **MySQL** 客户端库（可选，用于远程数据库）

### 编译步骤

使用QT5.15打开pro文件，编译

### 配置文件

项目使用以下配置文件：
- `naming_check_config.json` - 命名检查配置
- `score_config.json` - 评分配置
- `AnalyzeConfig.json` - 分析配置
- `Setting.json` - 用户设置

**以上配置文件+后端exe必须放置在QT生成成功之后的build\xxx（配套的Kit）-Debug文件夹中**

## 💻 使用说明

### 基本流程

1. **启动应用** - 运行 `AnalyzerClient.exe`
2. **创建/打开项目** - 通过欢迎页面创建新项目或打开现有项目
3. **选择分析目录** - 在文件管理器中浏览项目文件
4. **开始分析** - 点击分析按钮，等待分析完成
5. **查看结果** - 在代码编辑器中查看错误/警告标记，在评分组件中查看代码质量评分

### 主要功能

- **代码编辑** - 支持语法高亮、查找替换、自动完成
- **代码分析** - 自动检测代码错误、警告和潜在问题
- **克隆检测** - 识别重复代码片段
- **质量评分** - 从规范性、效率、可维护性、安全性四个维度评分
- **数据同步** - 支持本地和远程数据库同步

## 🔧 配置说明

### 分析配置

编辑 `AnalyzeConfig.json` 可以配置分析选项：
- 分析规则开关
- 警告级别设置
- 克隆检测阈值

### 评分配置

编辑 `score_config.json` 可以调整评分权重：
- 各维度权重分配
- 评分计算公式

## 📊 数据结构

主要数据结构定义在 `InfoStruct.h` 中：
- `IErrorInfo` - 错误信息
- `IWarningInfo` - 警告信息
- `ClonePosition` - 克隆位置
- `CloneSetInfo` - 克隆集合
- `GlobalInfo` - 全局统计信息
- `ScoreUnitType` - 评分维度类型



## 📧 联系方式

如有问题或建议，请通过 Issue 反馈。

---

**注意**：本项目依赖 Clang/LLVM 和 Boost 库，请确保这些库已正确安装和配置。
