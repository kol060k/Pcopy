#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <string>
#include <vector>

using namespace std;

vector<string> file_list;

void copy_dir(string source, string receiver) {	// Функция копирования дерева директорий из истоника в приёмник
	DIR *ds = opendir(source.c_str());
	DIR *dr = opendir(receiver.c_str());
	for (dirent *de = readdir(ds); de != NULL; de = readdir(ds)) {
		if (de->d_type == DT_DIR) {
			if (strcmp(de->d_name, ".") == 0) continue;
			if (strcmp(de->d_name, "..") == 0) continue;
			string new_source = source + "/" + de->d_name;
			string new_receiver = receiver + "/" + de->d_name;
			struct stat st;
			if (stat(new_receiver.c_str(), &st) == -1) {
				struct stat st1;
				stat(new_source.c_str(), &st1);
				mkdir(new_receiver.c_str(), st1.st_mode % 512);
			}
			copy_dir(new_source, new_receiver);
		}
		else {
			file_list.push_back(de->d_name);
		}
	}
	closedir(ds);
	closedir(dr);
}

int main(int argc, char** argv) {
	if (argc != 4) {			// Банальная проверка правильности ввода команды
		cout << "Wrong attributes" << endl;
		return 1;
	}
	int thread_number = atoi(argv[1]);	// Фиксируем количество thread'ов, с помощью которого будем копировать файлы

//// Создадим дерево директорий как в директории-источнике
	DIR *d = opendir(argv[2]);
	if (d == NULL) {
		perror(argv[2]);
		return 1;
	}
	closedir(d);
	d = opendir(argv[3]);
	if (d == NULL) {
		perror(argv[3]);
		return 1;
	}
	closedir(d);
	string source = argv[2];
	string receiver = argv[3];
	copy_dir(source, receiver);

//// Приступаем к копированию файлов
	
	return 0;
}
