# ===========================
# 🏦 Bank Server Build System
# ===========================

CC = gcc
CFLAGS = -Wall
INCLUDE = -Iinclude

# All source files for the server
SRC = src/server.c \
      src/handler.c \
      src/common.c \
      src/admin.c \
      src/admin_utils.c \
      src/manager.c \
      src/manager_utils.c \
      src/employee.c \
      src/employee_utils.c \
      src/customer.c \
      src/customer_utils.c

OUT = server

# New target for the session initializer
INIT_SESSIONS_SRC = src/init_sessions.c
INIT_SESSIONS_OUT = init_sessions

# ---------------------------
# 🧱 Default build target
# ---------------------------
all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(INCLUDE)

# ---------------------------
# 🛠️ Build the init_sessions tool
# ---------------------------
sessions_tool: $(INIT_SESSIONS_SRC)
	$(CC) $(CFLAGS) $(INIT_SESSIONS_SRC) -o $(INIT_SESSIONS_OUT) $(INCLUDE)

# ---------------------------
# 🧹 Clean build artifacts
# ---------------------------
clean:
	rm -f $(OUT) $(INIT_SESSIONS_OUT)
