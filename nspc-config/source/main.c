#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <switch.h>
#include <mbedtls/sha512.h>

#define SETTINGS_PATH "/config/parental_control/settings.json"

typedef struct {
    int enabled;
    int daily_limit;
    int show_remaining_time;
} NSPCSettings;

// 读取文件
char* read_file(const char* path, size_t* out_size) {
    FILE* fp = fopen(path, "r");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buf = malloc(size + 1);
    if (buf) {
        fread(buf, 1, size, fp);
        buf[size] = '\0';
        if (out_size) *out_size = size;
    }
    fclose(fp);
    return buf;
}

// 写入文件
int write_file(const char* path, const char* data) {
    FILE* fp = fopen(path, "w");
    if (!fp) return -1;
    int result = fputs(data, fp);
    fclose(fp);
    return result;
}

// 计算 SHA-512 hash
void calculate_hash(const char* data, char* out) {
    mbedtls_sha512_context ctx;
    unsigned char hash[64];

    mbedtls_sha512_init(&ctx);
    mbedtls_sha512_starts_ret(&ctx, 0);
    mbedtls_sha512_update_ret(&ctx, (const unsigned char*)data, strlen(data));
    mbedtls_sha512_finish_ret(&ctx, hash);
    mbedtls_sha512_free(&ctx);

    for (int i = 0; i < 64; i++) {
        sprintf(out + i * 2, "%02x", hash[i]);
    }
    out[128] = '\0';
}

// 生成 settings.json
void save_settings(NSPCSettings* settings) {
    // 构建不含 hash 的 JSON
    char json_no_hash[4096];
    snprintf(json_no_hash, sizeof(json_no_hash),
        "{\"settings\":["
        "{\"key\":\"enabled\",\"type\":0,\"value\":%d},"
        "{\"key\":\"daily_limit\",\"type\":0,\"value\":%d},"
        "{\"key\":\"show_remaining_time\",\"type\":0,\"value\":%d}"
        "]}",
        settings->enabled, settings->daily_limit, settings->show_remaining_time);

    // 计算 hash
    char hash[129];
    calculate_hash(json_no_hash, hash);

    // 构建完整 JSON（带 hash）
    char json[4096];
    snprintf(json, sizeof(json),
        "{\"settings\":["
        "{\"key\":\"enabled\",\"type\":0,\"value\":%d},"
        "{\"key\":\"daily_limit\",\"type\":0,\"value\":%d},"
        "{\"key\":\"show_remaining_time\",\"type\":0,\"value\":%d}"
        "],\"hash\":\"%s\"}",
        settings->enabled, settings->daily_limit, settings->show_remaining_time, hash);

    write_file(SETTINGS_PATH, json);
}

// 解析 settings.json（简化版）
void parse_settings(const char* json, NSPCSettings* settings) {
    // 默认值
    settings->enabled = 1;
    settings->daily_limit = 60;
    settings->show_remaining_time = 1;

    // 查找 "daily_limit"
    const char* p = strstr(json, "\"daily_limit\"");
    if (p) {
        p = strstr(p, "\"value\":");
        if (p) {
            settings->daily_limit = atoi(p + 8);
        }
    }

    // 查找 "enabled"
    p = strstr(json, "\"enabled\"");
    if (p) {
        p = strstr(p, "\"value\":");
        if (p) {
            settings->enabled = atoi(p + 8);
        }
    }

    // 查找 "show_remaining_time"
    p = strstr(json, "\"show_remaining_time\"");
    if (p) {
        p = strstr(p, "\"value\":");
        if (p) {
            settings->show_remaining_time = atoi(p + 8);
        }
    }
}

int main(int argc, char **argv) {
    consoleInit(NULL);

    // 初始化
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    NSPCSettings settings = {1, 60, 1};
    int menu_index = 0;
    const char* menu_items[] = {
        "Set daily limit",
        "Toggle enabled",
        "Toggle notifications",
        "Save & Exit",
        "Exit without saving"
    };
    const int menu_count = 5;

    // 读取当前配置
    char* json = read_file(SETTINGS_PATH, NULL);
    if (json) {
        parse_settings(json, &settings);
        free(json);
    }

    int running = 1;
    int saved = 0;

    while (appletMainLoop() && running) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        // 处理输入
        if (kDown & HidNpadButton_Up) {
            if (menu_index > 0) menu_index--;
        }
        if (kDown & HidNpadButton_Down) {
            if (menu_index < menu_count - 1) menu_index++;
        }

        if (kDown & HidNpadButton_A) {
            switch (menu_index) {
                case 0: { // Set daily limit
                    // 简单实现：直接加10分钟，到180分钟回到0
                    settings.daily_limit += 10;
                    if (settings.daily_limit > 180) settings.daily_limit = 0;
                    break;
                }
                case 1: { // Toggle enabled
                    settings.enabled = !settings.enabled;
                    break;
                }
                case 2: { // Toggle notifications
                    settings.show_remaining_time = !settings.show_remaining_time;
                    break;
                }
                case 3: { // Save & Exit
                    save_settings(&settings);
                    saved = 1;
                    running = 0;
                    break;
                }
                case 4: { // Exit without saving
                    running = 0;
                    break;
                }
            }
        }

        if (kDown & HidNpadButton_Plus) {
            running = 0;
        }

        // 渲染
        consoleClear();

        printf("\x1b[1;36m================================\n");
        printf("  NSPC Config Tool v1.0\n");
        printf("================================\x1b[0m\n\n");

        // 显示当前设置
        printf("Current Settings:\n");
        printf("  Daily Limit:  %d min\n", settings.daily_limit);
        printf("  Enabled:      %s\n", settings.enabled ? "Yes" : "No");
        printf("  Notifications: %s\n", settings.show_remaining_time ? "Yes" : "No");
        printf("\n-------------------------------\n\n");

        // 显示菜单
        printf("Use DPAD Up/Down + A to select\n");
        printf("Press + to exit\n\n");

        for (int i = 0; i < menu_count; i++) {
            if (i == menu_index) {
                printf("\x1b[1;32m> %s", menu_items[i]);
                // 显示额外信息
                if (i == 0) printf(" (%d min)", settings.daily_limit);
                else if (i == 1) printf(" (%s)", settings.enabled ? "Yes" : "No");
                else if (i == 2) printf(" (%s)", settings.show_remaining_time ? "Yes" : "No");
                printf("\x1b[0m\n");
            } else {
                printf("  %s", menu_items[i]);
                if (i == 0) printf(" (%d min)", settings.daily_limit);
                else if (i == 1) printf(" (%s)", settings.enabled ? "Yes" : "No");
                else if (i == 2) printf(" (%s)", settings.show_remaining_time ? "Yes" : "No");
                printf("\n");
            }
        }

        printf("\n\n");
        if (saved) {
            printf("\x1b[1;32m[Settings saved!]\x1b[0m\n");
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
