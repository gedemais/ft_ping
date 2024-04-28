NAME=ft_ping
INET_PING=inet_ping

CC=gcc
CFLAGS=-Wall -Werror -Wextra -std=gnu99

SRC=src/main.c\
	src/utils.c

OBJ=$(SRC:.c=.o)

INC_PATH=include/
INC=include/main.h

all: $(INET_PING) $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

$(SRC_PATH)%.o : $(SRC_PATH)%.c $(INC)
	$(CC) $(CFLAGS) -I$(INC_PATH) -o $@ -c $<

$(INET_PING):
	bash scripts/install_inetutils_2.0.sh

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME) $(INET_PING)

re: fclean all
