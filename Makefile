# @file MakeFile
# @brief MakeFile for ISA LDAP server project
# @author Filip Polomski
# @date 2023-10-5

CC := g++
FLAGS := -Wall -Wextra -g
INCLUDE_DIR := include
SRC_DIR := src
BUILD_DIR := build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

all: $(SRCS) $(HEADERS)
		@$(CC) $(FLAGS) $(SRCS) -o isa_ldapserver 

clean:
		@$(RM) -f isa_ldapserver