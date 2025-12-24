# LOL 配置助手 (YiKkLOLSettingAssistant)

备份、还原、删除国服英雄联盟配置文件 `PersistedSettings.json`，并自动搜索安装路径。

## 功能
- 自动定位 `英雄联盟\Game\Config\PersistedSettings.json`（先查常见路径，找不到再全盘搜索）。
- 输入 `1` 备份：将配置保存到 `%APPDATA%/YiKkLOLSettingAssistant/`（若无法获取 APPDATA，则退回当前目录/隐藏文件夹）。
- 输入 `2` 还原：用已备份的文件覆盖游戏配置。
- 输入 `3` 删除：删除备份文件及其文件夹（含二次确认）。

## 使用
1) 编译或直接运行已编译的 `LOLConfigSaver.exe`。
2) 按提示输入 1/2/3/0 进行操作。

## 编译示例
```bash
g++ -std=c++17 -O2 -s LOLConfigSaver.cpp -o LOLConfigSaver.exe
```

## 提示
- 如还原或删除失败，请尝试以管理员身份运行。
- 首次运行若未找到备份，会提示备份将保存到的路径。
