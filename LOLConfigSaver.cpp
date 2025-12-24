#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include <cstdlib>

namespace fs = std::filesystem;

// 配置文件名与路径特征
const std::string CONFIG_FILENAME   = "PersistedSettings.json";
const std::string PATH_PATTERN_PART = "Game\\Config";
const std::string LOL_FOLDER_NAME   = "英雄联盟";
const std::string BACKUP_DIR_NAME   = "YiKkLOLSettingAssistant";

// 设置控制台编码为 UTF-8 并修改窗口标题
void setupConsole() {
    SetConsoleOutputCP(65001);
    SetConsoleTitleA("YiKkLOLSettingAssistant");
    std::setlocale(LC_ALL, ".UTF8");
}

// 计算备份文件的完整路径；优先存放在 APPDATA\BACKUP_DIR_NAME 下
fs::path getBackupPath() {
    fs::path backupDir;
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        backupDir = fs::path(appdata) / BACKUP_DIR_NAME;
    } else {
        // 如果获取不到 APPDATA，回退到当前目录下的隐藏文件夹
        backupDir = fs::current_path() / ("." + BACKUP_DIR_NAME);
    }

    if (!fs::exists(backupDir)) {
        try {
            fs::create_directories(backupDir);
        } catch (...) {
            // 回退到当前目录，至少保证有可写路径
            backupDir = fs::current_path();
        }
    }
    
    return backupDir / CONFIG_FILENAME;
}

// 获取所有逻辑驱动器
std::vector<std::string> getLogicalDrives() {
    std::vector<std::string> drives;
    DWORD driveMask = GetLogicalDrives();
    for (int i = 0; i < 26; ++i) {
        if (driveMask & (1 << i)) {
            std::string drive = "";
            drive += (char)('A' + i);
            drive += ":\\";
            drives.push_back(drive);
        }
    }
    return drives;
}

// 检查路径是否包含目标配置文件
fs::path checkPath(const fs::path& lolPath) {
    fs::path fullPath = lolPath / PATH_PATTERN_PART / CONFIG_FILENAME;
    if (fs::exists(fullPath)) {
        return fullPath;
    }
    return "";
}

// 搜索配置文件路径
fs::path findLOLConfigPath() {
    std::cout << "正在搜索英雄联盟配置文件，请稍候..." << std::endl;

    std::vector<std::string> drives = getLogicalDrives();

    // 1. 优先搜索常见路径 (快速模式)
    std::vector<std::string> commonPaths = {
        "英雄联盟",
        "Program Files\\英雄联盟",
        "Program Files (x86)\\英雄联盟",
        "WeGameApps\\英雄联盟",
        "Software\\WeGame\\Apps\\英雄联盟"
    };

    for (const auto& drive : drives) {
        for (const auto& sub : commonPaths) {
            fs::path p = fs::path(drive) / sub;
            fs::path result = checkPath(p);
            if (!result.empty()) return result;
        }
    }

    // 2. 如果常见路径没找到，进行全盘搜索 (慢速模式)
    std::cout << "常见路径未找到，正在进行全盘深度搜索（可能需要几分钟）..." << std::endl;
    
    for (const auto& drive : drives) {
        try {
            // 使用 recursive_directory_iterator 遍历
            // skip_permission_denied 避免权限错误
            for (auto& p : fs::recursive_directory_iterator(drive, fs::directory_options::skip_permission_denied)) {
                try {
                    if (p.is_directory() && p.path().filename() == LOL_FOLDER_NAME) {
                        fs::path result = checkPath(p.path());
                        if (!result.empty()) return result;
                    }
                } catch (...) {
                    continue;
                }
            }
        } catch (...) {
            continue;
        }
    }

    return "";
}

void backupConfig(const fs::path& configPath) {
    std::cout << "您选择了保存配置。" << std::endl;
    std::cout << "请再次输入 1 确认保存（将覆盖已有的备份）：";
    int confirm;
    if (!(std::cin >> confirm)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        confirm = 0;
    }

    if (confirm == 1) {
        try {
            fs::path backupPath = getBackupPath();
            fs::copy_file(configPath, backupPath, fs::copy_options::overwrite_existing);
            std::cout << "✅ 配置已成功保存到: " << backupPath.string() << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cout << "❌ 保存失败: " << e.what() << std::endl;
        }
    } else {
        std::cout << "操作已取消。" << std::endl;
    }
}

void restoreConfig(const fs::path& configPath) {
    fs::path backupPath = getBackupPath();
    if (!fs::exists(backupPath)) {
        std::cout << "❌ 未找到已保存的配置文件。" << std::endl;
        return;
    }

    std::cout << "正在将备份应用到: " << configPath.string() << std::endl;
    try {
        fs::copy_file(backupPath, configPath, fs::copy_options::overwrite_existing);
        std::cout << "✅ 配置已成功应用！" << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cout << "❌ 应用失败: " << e.what() << std::endl;
        std::cout << "请尝试以管理员身份运行此程序。" << std::endl;
    }
}

void deleteBackup() {
    fs::path backupFile = getBackupPath();
    fs::path backupDir = backupFile.parent_path();

    if (!fs::exists(backupDir)) {
        std::cout << "ℹ️ 未发现备份文件夹，无需删除。" << std::endl;
        return;
    }

    std::cout << "⚠️ 您确定要删除所有备份数据吗？(包括文件夹 " << backupDir.string() << ")" << std::endl;
    std::cout << "请再次输入 1 确认删除：";
    int confirm;
    if (!(std::cin >> confirm)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        confirm = 0;
    }

    if (confirm == 1) {
        try {
            fs::remove_all(backupDir);
            std::cout << "✅ 已成功删除备份文件夹及内容。" << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cout << "❌ 删除失败: " << e.what() << std::endl;
        }
    } else {
        std::cout << "操作已取消。" << std::endl;
    }
}

int main() {
    setupConsole();

    fs::path configPath = findLOLConfigPath();

    if (configPath.empty()) {
        std::cout << "❌ 未能自动找到英雄联盟配置文件路径。" << std::endl;
        std::cout << "请确保游戏已安装且路径包含 '英雄联盟\\Game\\Config'。" << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "✅ 已找到配置文件: " << configPath.string() << std::endl;
    
    fs::path backupPath = getBackupPath();
    if (fs::exists(backupPath)) {
        std::cout << "📂 检测到已备份配置: " << backupPath.string() << std::endl;
    } else {
        std::cout << "ℹ️ 当前无备份，备份将保存至: " << backupPath.string() << std::endl;
    }
    
    std::cout << "--------------------------------------------------" << std::endl;

    while (true) {
        std::cout << "\n请选择操作：" << std::endl;
        std::cout << "1. 保存当前配置文件 (备份)" << std::endl;
        std::cout << "2. 应用保存的配置文件 (覆盖)" << std::endl;
        std::cout << "3. 删除已保存的备份和文件夹" << std::endl;
        std::cout << "0. 退出" << std::endl;
        std::cout << "输入: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "无效输入，请输入数字。" << std::endl;
            continue;
        }

        if (choice == 1) {
            backupConfig(configPath);
        } else if (choice == 2) {
            restoreConfig(configPath);
        } else if (choice == 3) {
            deleteBackup();
        } else if (choice == 0) {
            break;
        } else {
            std::cout << "无效的选择。" << std::endl;
        }
    }

    return 0;
}
