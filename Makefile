SRC=		src/utils/Hash.cpp			\
		src/utils/ConfigManager.cpp		\
		src/utils/Dev.cpp			\
		src/utils/Config.cpp			\
		src/utils/StringParser.cpp		\
		src/utils/Log.cpp			\
		src/network/RawSocket.cpp		\
		src/network/ProgressBuffer.cpp		\
		src/network/ByteBuffer.cpp		\
		src/network/TCPClient.cpp		\
		src/network/TCPClientSSL.cpp		\
		src/network/ASocket.cpp			\
		src/network/WebSocket.cpp		\
		src/network/TCPListener.cpp		\
		src/network/TCPListenerSSL.cpp		\
		src/network/Select.cpp			\

OBJ=		$(SRC:.cpp=.o)

INCLUDE=	-I include/				\
		-I include/network/			\
		-I include/utils/			\
		-I include/thread/			\

CXXFLAGS=	-g -Wall -Wshadow -Wextra -std=c++11 $(INCLUDE)

NAME=		LibYo.a

all:		$(NAME)

$(NAME):	$(OBJ)
		ar rc $(NAME) $(OBJ)

clean:
		rm -f $(OBJ)

fclean:		clean
		rm -f $(NAME)

re:		fclean all
