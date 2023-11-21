# @file MakeFile
# @author Filip Polomski, xpolom00
# @brief MakeFile for ISA LDAP server project
# @version 1.0
# @date 2023-10-5

CC := g++
FLAGS := -Wall -Wextra -g
INCLUDE_DIR := include
SRC_DIR := src
BUILD_DIR := build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

all: $(SRCS) $(HEADERS)
		@$(CC) $(FLAGS) $(SRCS) -o isa-ldapserver 

clean:
		@$(RM) -f isa-ldapserver