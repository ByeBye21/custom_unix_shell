// #21. Grup (SISTEM MIMARLARI )
// B221210588 Younes Rahebi 1-C
// B221210090 Umut Direk 1-C
// B221210577 Nurcan Ahmet 1-A
// B221210070 Şevval Nur Kalaycı 1-A
// B221210055 Şeyma Orhan 1-A

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CMD_LEN 1024 // Maksimum komut uzunluğu
#define MAX_CMD_ARGS 100 // Maksimum komut argümanı sayısı
#define MAX_COMMANDS 10  // Aynı anda maksimum 10 komut çalıştırabilir
#define MAX_BUFFER 1024  // Kullanıcıdan gelen komutların maksimum uzunluğu
#define MAX_BG_PROCS 100 // Maksimum arka plan işlemi

typedef struct {
    pid_t pid; // İşlem PID'si
    int active; // İşlem aktif mi?
} BackgroundProcess;

BackgroundProcess bg_processes[MAX_BG_PROCS]; // Arka plan işlemleri dizisi
int bg_count = 0; // Aktif arka plan işlemleri sayısı

void sigchld_handler(int signum) {
    (void)signum; // Kullanılmadı
    int status;
    pid_t pid;

    // Tamamlanan arka plan işlemleri için bekle
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_count; i++) {
            if (bg_processes[i].pid == pid) {
                bg_processes[i].active = 0; // İşlem tamamlandı
                printf("[%d] retval: %d\n", pid, WEXITSTATUS(status));
                fflush(stdout);
                break;
            }
        }
    }
}

// Prompt yazdıran fonksiyon
void print_prompt() {
    printf("> ");
    fflush(stdout);
}

// Kullanıcıdan komut okuma fonksiyonu
void read_command(char *command) {
    if (fgets(command, MAX_CMD_LEN, stdin) != NULL) {
        command[strcspn(command, "\n")] = 0; // Yeni satır karakterini kaldır
    }
}

// Komutun parametrelerini ayrıştıran fonksiyon
int parse_command(char *command, char *args[]) {
    int i = 0;
    args[i] = strtok(command, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
    return i; // Parametre sayısını döndür
}

void giris_yonlendir(char *komut) {
    char *args[MAX_CMD_ARGS];
    int i = 0;

    // Komut ve yönlendirme dosyası adı ayrıştırılıyor
    args[i] = strtok(komut, "<");
    while (args[i] != NULL && i < 2) {
        i++;
        args[i] = strtok(NULL, "<");
    }

    if (args[1] == NULL) {
        fprintf(stderr, "Giriş yönlendirme dosyası belirtilmedi.\n");
        return;
    }

    // Dosya adının başındaki/bitişindeki boşlukları kaldır
    char *dosya_adi = args[1];
    while (*dosya_adi == ' ') dosya_adi++; // Başındaki boşlukları at
    dosya_adi[strcspn(dosya_adi, "\n")] = 0; // Yeni satır karakterini kaldır

    int dosya = open(dosya_adi, O_RDONLY);
    if (dosya == -1) {
        perror("Giriş dosyası açılamadı");
        return;
    }

    int stdin_backup = dup(STDIN_FILENO); // stdin'i yedekle
    dup2(dosya, STDIN_FILENO);
    close(dosya);

    // İlk parça olan komut çalıştırılıyor
    system(args[0]);

    dup2(stdin_backup, STDIN_FILENO); // stdin'i geri yükle
    close(stdin_backup);
}


void cikis_yonlendir(char *komut) {
    char *args[MAX_CMD_ARGS];
    int i = 0;

    // Komut ve yönlendirilecek dosya adı ayrıştırılıyor
    args[i] = strtok(komut, ">");
    while (args[i] != NULL && i < 2) {
        i++;
        args[i] = strtok(NULL, ">");
    }

    if (args[1] == NULL) {
        fprintf(stderr, "Yönlendirme dosyası belirtilmedi.\n");
        return;
    }

    // Dosya adının başındaki/bitişindeki boşlukları kaldır
    char *dosya_adi = args[1];
    while (*dosya_adi == ' ') dosya_adi++; // Başındaki boşlukları at
    dosya_adi[strcspn(dosya_adi, "\n")] = 0; // Yeni satır karakterini kaldır

    int dosya = open(dosya_adi, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dosya == -1) {
        perror("Çıkış dosyası açılamadı");
        return;
    }

    int stdout_backup = dup(STDOUT_FILENO); // stdout'u yedekle
    dup2(dosya, STDOUT_FILENO);
    close(dosya);

    // İlk parça olan komut çalıştırılıyor
    system(args[0]);

    dup2(stdout_backup, STDOUT_FILENO); // stdout'u geri yükle
    close(stdout_backup);
}



// Tek bir komutu pipe ile çalıştıran fonksiyon
static int command(int input, int first, int last, char *cmd_exec) {
    int mypipefd[2], ret;
    pid_t pid;

    ret = pipe(mypipefd);
    if (ret == -1) {
        perror("Pipe oluşturulamadı");
        return 1;
    }

    pid = fork();
    if (pid == 0) {
        if (first && !last && input == 0) {
            dup2(mypipefd[1], STDOUT_FILENO);
        } else if (!first && !last && input != 0) {
            dup2(input, STDIN_FILENO);
            dup2(mypipefd[1], STDOUT_FILENO);
        } else {
            dup2(input, STDIN_FILENO);
        }

        close(mypipefd[0]);

        char *args[MAX_CMD_ARGS];
        int i = 0;
        args[i] = strtok(cmd_exec, " ");
        while (args[i] != NULL) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (execvp(args[0], args) < 0) {
            perror("Komut çalıştırılamadı");
            exit(1);
        }
    } else {
        waitpid(pid, NULL, 0);
    }

    if (last) close(mypipefd[0]);
    if (input != 0) close(input);
    close(mypipefd[1]);

    return mypipefd[0];
}

// Pipe içeren komutları çalıştıran fonksiyon
void with_pipe_execute(char *input_buffer) {
    int i, n = 1, input = 0, first = 1;
    char *cmd_exec[MAX_COMMANDS];

    cmd_exec[0] = strtok(input_buffer, "|");
    while ((cmd_exec[n] = strtok(NULL, "|")) != NULL) n++;
    cmd_exec[n] = NULL;

    for (i = 0; i < n - 1; i++) {
        input = command(input, first, 0, cmd_exec[i]);
        first = 0;
    }
    command(input, first, 1, cmd_exec[i]);
}

void with_semicolon_execute(char *input_buffer) {
    char *cmd_exec[MAX_COMMANDS];
    int n = 0;

    // Komutları ";" karakterine göre böl
    cmd_exec[0] = strtok(input_buffer, ";");
    while ((cmd_exec[++n] = strtok(NULL, ";")) != NULL);

    // Her bir komutu sırayla çalıştır
    for (int i = 0; i < n; i++) {
        char *cmd = cmd_exec[i];
        while (*cmd == ' ') cmd++; // Başındaki boşlukları atla
        cmd[strcspn(cmd, "\n")] = 0; // Sonundaki yeni satırları kaldır

        if (strlen(cmd) == 0) continue; // Boş komutları atla

        pid_t pid = fork();

        if (pid == -1) {
            perror("Fork başarısız oldu");
        } else if (pid == 0) {
            char *args[MAX_CMD_ARGS];
            parse_command(cmd, args);

            // Komut çalıştır
            if (execvp(args[0], args) == -1) {
                perror("Komut çalıştırılamadı");
            }
            exit(1);
        } else {
            waitpid(pid, NULL, 0); // Çocuk işlemi bekle
        }
    }
}

// Yeni arka plan işlemi ekleme
void add_background_process(pid_t pid) {
    if (bg_count < MAX_BG_PROCS) {
        bg_processes[bg_count].pid = pid;
        bg_processes[bg_count].active = 1;
        bg_count++;
    } else {
        fprintf(stderr, "Maksimum arka plan işlemi sınırına ulaşıldı.\n");
    }
}

// Tüm arka plan işlemlerini bekle
void wait_for_all_bg_processes() {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].active) {
            waitpid(bg_processes[i].pid, NULL, 0);
        }
    }
}

void increment(const char *filename) {
    FILE *file = NULL;
    int number = 0;

    if (filename != NULL && strlen(filename) > 0) {
        file = fopen(filename, "r+");
        if (file == NULL) {
            perror("Dosya açılamadı");
            return;
        }

        if (fscanf(file, "%d", &number) != 1) {
            printf("Dosyadan sayı okunamadı, başlangıç olarak 0 kabul ediliyor.\n");
        }
    } else {
        // Standart girdiden oku
        if (scanf("%d", &number) != 1) {
            printf("Standart girdiden sayı okunamadı, başlangıç olarak 0 kabul ediliyor.\n");
        }
    }

    number++;

    if (file != NULL) {
        rewind(file); // Dosya imlecini başa al
        fprintf(file, "%d\n", number);
        fclose(file);
    }

    printf("%d\n", number);
}


// Ana program
int main() {
    char command[MAX_CMD_LEN];
    char *args[MAX_CMD_ARGS];
    pid_t pid;
    int status;

    // SIGCHLD sinyali için işleyici tanımla
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    while (1) {
        print_prompt();
        read_command(command);

        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            printf("Shell sonlandırılıyor, tüm arka plan işlemleri bekleniyor...\n");
            wait_for_all_bg_processes();
            break;
        }

        if (strncmp(command, "increment", 9) == 0) {
            char *filename = command + 10; // Komuttan dosya adını al
            while (*filename == ' ') filename++; // Boşlukları atla
            if (strlen(filename) == 0) {
                filename = NULL; // Dosya adı belirtilmediyse NULL geç
            }
            increment(filename);
            continue;
        }

        int bg_flag = 0; // Arka plan işareti
        if (command[strlen(command) - 1] == '&') {
            bg_flag = 1;
            command[strlen(command) - 1] = '\0'; // '&' işaretini kaldır
        }

        if (strstr(command, "<") != NULL) {
            giris_yonlendir(command);
        } else if (strstr(command, ">") != NULL) {
            if (strchr(command, '>') == command + strlen(command) - 1) {
            fprintf(stderr, "Geçersiz yönlendirme: Dosya adı belirtilmemiş.\n");
            continue;
    	}
    	    cikis_yonlendir(command);
        } else if (strstr(command, "|") != NULL) {
            with_pipe_execute(command);
        } else if (strstr(command, ";") != NULL) {
            with_semicolon_execute(command);
        } else {
            pid = fork();
            if (pid == -1) {
                perror("Fork başarısız oldu");
            } else if (pid == 0) {
                parse_command(command, args);
                if (execvp(args[0], args) == -1) {
                    perror("Komut çalıştırılamadı");
                }
                exit(1);
            } else {
                if (bg_flag) {
                    printf("[%d] başladı\n", pid);
                    fflush(stdout);
                    add_background_process(pid);
                } else {
                    waitpid(pid, &status, 0);
                }
            }
        }
    }

    return 0;
}
