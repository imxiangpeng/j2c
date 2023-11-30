

gcc j2c.c j2cobject.c -I. `pkg-config --cflags libcjson` `pkg-config --libs libcjson` -o j2c
# 查看运行结果
./j2c
valgrind --leak-check=full ./j2c



