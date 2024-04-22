NAME=ft_ping

CC=gcc
CFLAGS=-Wall -Werror -Wextra -g3 -fsanitize=address

SRC=src/main.c

OBJ=$(SRC:.c=.o)

INC=include/main.h

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ)

$(SRC_PATH)%.o : $(SRC_PATH)%.c $(INC)
	$(CC) $(CFLAGS) -I$(INC_PATH) -o $@ -c $<

clean:
	rm -rf $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all
