NAME	= pamela_module.so

CC	= gcc

RM	= rm -rf

CP	= cp

MKDIR	=	mkdir -p

SRCS	= src/pamela_module.c

OBJS	= $(SRCS:.c=.o)

CFLAGS = -W -Wall -Wextra -Werror -fPIC -DPIC -shared -rdynamic

all: $(NAME)

$(NAME): $(OBJS)
	 $(CC) $(OBJS) -libcryptsetup -o $(NAME)

install:
ifneq ("$(wildcard /lib/security)", "")
	@printf "\033[0;31mPAM module already installed\n\033[0m"
else
	apt-get install -y libcryptsetup-dev libpam0g-dev gcc
	make
	@$(MKDIR) /lib/security
	@$(CP) $(NAME) /lib/security/$(NAME)
	@echo "auth optional pamela_module.so" >> /etc/pam.d/common-auth
	@echo "session optional pamela_module.so" >> /etc/pam.d/common-session
	@printf "\033[0;32mPAM module installed successfully\n\033[0m"
endif

uninstall:
ifeq ("$(wildcard /lib/security)", "")
	@printf "\033[0;31mPAM module not installed\n\033[0m"
else
	make clean
	@$(RM) /lib/security
	@head -n -1 /etc/pam.d/common-auth > /tmp/common-auth && mv /tmp/common-auth /etc/pam.d/common-auth
	@head -n -1 /etc/pam.d/common-session > /tmp/common-session && mv /tmp/common-session /etc/pam.d/common-session
	@printf "\033[0;32mPAM module uninstalled successfully\n\033[0m"
endif

check:
ifeq ("$(wildcard /lib/security)", "")
	@printf "\033[0;31mPAM module not installed\n\033[0m"
else
	@printf "\033[0;32mPAM module installed\n\033[0m"
endif

test:

clean:
	$(RM) $(OBJS)
	$(RM) $(NAME)

re: clean all
