#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Định nghĩa cấu trúc tài khoản
struct Account {
    char username[10];
    int status;
};

// Hàm kiểm tra xem username có tồn tại trong file không
int isUsernameExist(const char* username) {
    FILE* file = fopen("account.txt", "r");
    if (file == NULL) {
        printf("Khong the mo file account.txt.\n");
        exit(1);
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char storedUsername[50];
        int status;
        sscanf(line, "%s %d", storedUsername, &status);
        if (strcmp(username, storedUsername) == 0) {
            fclose(file);
            return status;
        }
    }

    fclose(file);
    return -1;  // Trả về -1 nếu không tìm thấy username
}

// Hàm ghi nhật ký hoạt động vào file log
void logActivity(int choice, const char* username, int result) {
    time_t t;
    struct tm* now;
    char timestamp[50];
    char logFileName[50];
    char logLine[100];

    // Lấy thời gian hiện tại
    time(&t);
    now = localtime(&t);
    strftime(timestamp, sizeof(timestamp), "[%d/%m/%Y %H:%M:%S]", now);

    // Tạo tên file log 
    strftime(logFileName, sizeof(logFileName), "log_20204973.txt", now);

    // Mở file log để ghi
    FILE* logFile = fopen(logFileName, "a");
    if (logFile == NULL) {
        printf("Khong the mo  file log.\n");
        exit(1);
    }

    // Ghi nhật ký hoạt động vào file log
    sprintf(logLine, "%s $ %d $ %s $ %s\n", timestamp, choice, username, (result == 1) ? "+OK" : "-ERR");
    fputs(logLine, logFile);

    fclose(logFile);
}

int main() {
    int choice; 
    char username[50];
    int isLoggedIn = 0; 
    // Biến kiểm tra tài khoản đã đăng nhập hay chưa

    while (1) {
        printf("Menu:\n");
        printf("1. Log in\n");
        printf("2. Post message\n");
        printf("3. Logout\n");
        printf("4. Exit\n");
        printf("Please choose functions from 1 to 4: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (isLoggedIn) {
                    printf("Ban da dang nhap bang tai khoan khac. Hay dang xuat truoc.\n");
                } else {
                    printf("Input username: ");
                    scanf("%s", username);

                    int status = isUsernameExist(username);
                    if (status == 1) {
                        printf("Dang nhap thanh cong!\n");
                        isLoggedIn = 1;
                        logActivity(1, username, 1); // Ghi nhật ký đăng nhập thành công
                    } else if (status == 0) {
                        printf("Tai khoan da bi khoa. Dang nhap that bai.\n");
                        logActivity(1, username, -1); // Ghi nhật ký đăng nhập thất bại
                    } else {
                        printf("Tai khoan da ton tai. Dang nhap that bai!\n");
                        logActivity(1, username, -1); // Ghi nhật ký đăng nhập thất bại
                    }
                }
                break;
            case 2:
                if (isLoggedIn) {
                    printf("Nhap bai viet: ");
                    char message[100];
                    scanf(" %[^\n]", message);
                    printf("Dang bai thanh cong.\n");
                    logActivity(2, username, 1); // Ghi nhật ký đăng bài thành công
                } else {
                    printf("Ban can dang nhap truoc khi dang bai.\n");
                    logActivity(2, username, -1); // Ghi nhật ký đăng bài thất bại
                }
                break;
            case 3:
                if (isLoggedIn) {
                    isLoggedIn = 0;
                    printf("Dang xuat thanh cong.\n");
                    logActivity(3, username, 1); // Ghi nhật ký đăng xuất thành công
                } else {
                    printf("Ban chua dang nhap.\n");
                    logActivity(3, username, -1); // Ghi nhật ký đăng xuất thất bại
                }
                break;
            case 4:
                printf("Chuong trinh ket thuc.\n");
                exit(0);
            default:
                printf("Chuc nang khong hop le.\n");
        }
    }

    return 0;
}
