NAME=ft_ping
INET_PING=ping

CC=gcc
CFLAGS=-Wall -Werror -Wextra -std=gnu99 -g3

SRC=src/main.c\
	src/send.c\
	src/utils.c\
	src/stats.c

OBJ=$(SRC:.c=.o)

INC_PATH=include/
INC=include/main.h

all: $(INET_PING) $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

$(SRC_PATH)%.o : $(SRC_PATH)%.c $(INC)
	$(CC) $(CFLAGS) -I$(INC_PATH) -o $@ -c $<

$(INET_PING):
	@bash scripts/install_inetutils_2.0.sh

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME) $(INET_PING) inetutils-2.0.tar.xz

re: fclean all
