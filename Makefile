NAME=ft_ping

CC=gcc
CFLAGS=-Wall -Werror -Wextra -g3 -fsanitize=address

OBJ=$(SRC:.c=.o)

all: $(LIB) $(NAME)

$(LIB): $(LIB_PATH)
	make -C $(LIB_PATH)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LIB) -lm 

$(SRC_PATH)%.o : $(SRC_PATH)%.c $(INC)
	$(CC) $(CFLAGS) -I$(INC_PATH) -I$(LIB_PATH) -o $@ -c $<

clean:
	rm -rf $(OBJ)
	make -C $(LIB_PATH) clean

fclean: clean
	rm -rf $(NAME)
	make -C $(LIB_PATH) fclean

re: fclean all
