// #21. Grup (SISTEM MIMARLARI )
// B221210588 Younes Rahebi 1-C
// B221210090 Umut Direk 1-C
// B221210577 Nurcan Ahmet 1-A
// B221210070 Şevval Nur Kalaycı 1-A
// B221210055 Şeyma Orhan 1-A


#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CMD_LEN 1024 // Maksimum komut uzunluğu
#define MAX_CMD_ARGS 100 // Maksimum komut argümanı sayısı
#define MAX_COMMANDS 10  // Aynı anda maksimum 10 komut çalıştırılabilir
#define MAX_BUFFER 1024  // Kullanıcıdan gelen komutların maksimum uzunluğu
#define MAX_BG_PROCS 100 // Maksimum arka plan işlemi sayısı

// Arka plan işlem yapısı
typedef struct
{
    pid_t pid;  // İşlem PID'si
    int active; // İşlem aktif mi?
} BackgroundProcess;

// Arka plan işlemleri dizisi
extern BackgroundProcess bg_processes[MAX_BG_PROCS];
extern int bg_count; // Aktif arka plan işlemleri sayısı

// SigCHLD sinyali işleyicisi
void sigchld_handler(int signum);

// Prompt yazdıran fonksiyon
void print_prompt();

// Kullanıcıdan komut okuma fonksiyonu
void read_command(char *command);

// Komutun parametrelerini ayrıştıran fonksiyon
int parse_command(char *command, char *args[]);

// Giriş yönlendirmesi yapmak için fonksiyon
void giris_yonlendir(char *komut);

// Çıkış yönlendirmesi yapmak için fonksiyon
void cikis_yonlendir(char *komut);

// Tek bir komutu pipe ile çalıştıran fonksiyon
static int command(int input, int first, int last, char *cmd_exec);

// Pipe içeren komutları çalıştıran fonksiyon
void with_pipe_execute(char *input_buffer);

// Birden fazla komut içeren ve ';' ile ayrılmış komutları çalıştıran fonksiyon
void with_semicolon_execute(char *input_buffer);

// Yeni bir arka plan işlemi ekleme fonksiyonu
void add_background_process(pid_t pid);

// Tüm arka plan işlemlerini bekleme fonksiyonu
void wait_for_all_bg_processes();

// Dosya okuma veya kullanıcı girdisinden sayı arttırma fonksiyonu
void increment(const char *filename);

#endif // SHELL_H
