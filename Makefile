# Makefile
#21. Grup (SISTEM MIMARLARI )

#B221210577 Nurcan Ahmet 1-A
#B221210070 Şevval Nur Kalaycı 1-A
#B221210055 Şeyma Orhan 1-A
#B221210588 Younes Rahebi 1-C
#B221210090 Umut Direk 1-C


# Derleyici
CC = gcc

# Derleme bayrakları
CFLAGS = -Wall -g -std=c99

# Kaynak dosyaları ve başlık dosyaları
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

# Kaynak dosyaları
SRC_FILES = $(SRC_DIR)/shell.c

# Nesne dosyaları
OBJ_FILES = $(OBJ_DIR)/shell.o

# Çalıştırılabilir dosya
EXEC = $(BIN_DIR)/shell

# Varsayılan hedef (shell)
all: $(OBJ_DIR) $(BIN_DIR) $(EXEC)

# Shell çalıştırılabilir dosyasını oluştur
$(EXEC): $(OBJ_FILES)
	$(CC) $(OBJ_DIR)/*.o -o $(EXEC)

# Nesne dosyalarını derle
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Dizinleri oluştur
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# Temizleme komutu
clean:
	rm -rf $(OBJ_DIR)/*.o $(BIN_DIR)/shell

# Yeniden derleme komutu
rebuild: clean all
