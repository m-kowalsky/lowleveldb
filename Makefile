TARGET = bin/dbview
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))

run: clean default
	./$(TARGET) -f ./mynewdb.db -n
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Cheshire ln.,120"
	./$(TARGET) -f ./mynewdb.db -a "Barb B.,321 Cheshire ln.,110"
	./$(TARGET) -f ./mynewdb.db -a "Mikey K.,456 Cheshire ln.,100"
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -u "Timmy H.,200"
	./$(TARGET) -f ./mynewdb.db -u "Mikey K.,270"
	./$(TARGET) -f ./mynewdb.db -u "Barb B.,300"
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -r "Timmy H."
	./$(TARGET) -f ./mynewdb.db -l
	./$(TARGET) -f ./mynewdb.db -a "Timmy H.,123 Cheshire ln.,120"
	./$(TARGET) -f ./mynewdb.db -l





default: $(TARGET)

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET) : $(OBJ)
	gcc -o $@ $?

obj/%.o : src/%.c
	gcc -c $< -o $@ -Iinclude
	
