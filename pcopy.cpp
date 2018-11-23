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

struct copy_info {
	string from;
	string to;
};

struct pthread_data {
	pthread_mutex_t* mutex;
	vector<copy_info>* file_list;
};

void copy_dir(string source, string receiver, vector<copy_info>* file_list) {
// Функция копирования дерева директорий из истоника в приёмник с добавлением найденных файлов в file_list
	DIR *ds = opendir(source.c_str());
	DIR *dr = opendir(receiver.c_str());
	for (dirent *de = readdir(ds); de != NULL; de = readdir(ds)) {
		string new_source = source + "/" + de->d_name;
		string new_receiver = receiver + "/" + de->d_name;
		if (de->d_type == DT_DIR) {
			if (strcmp(de->d_name, ".") == 0) continue;
			if (strcmp(de->d_name, "..") == 0) continue;
			struct stat st;
			if (stat(new_receiver.c_str(), &st) == -1) {
				struct stat st1;
				stat(new_source.c_str(), &st1);
				mkdir(new_receiver.c_str(), st1.st_mode % 512);
			}
			copy_dir(new_source, new_receiver, file_list);
		}
		else {
			copy_info new_file;
			new_file.from = new_source;
			new_file.to = new_receiver;
			file_list->push_back(new_file);
		}
	}
	closedir(ds);
	closedir(dr);
}

/*void* file_copy(void* data) {

}*/

int main(int argc, char** argv) {
	if (argc != 4) {			// Банальная проверка правильности ввода команды
		cout << "Wrong attributes" << endl;
		return 1;
	}
	int thread_number = atoi(argv[1]);	// Фиксируем количество thread'ов, с помощью которого будем копировать файлы

//// Создадим дерево директорий как в директории-источнике
	vector<copy_info> file_list;
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
	copy_dir(source, receiver, &file_list);
	for (int i = 0; i < file_list.size(); i++)
		cout << file_list[i].from << " -> " << file_list[i].to << endl;

//// Приступаем к копированию файлов
/*	pthread_data data[thread_number];
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, NULL);
	for (int i = 0; i < thread_number; i++) {
		data[i].file_list = &file_list;
		data[i].mutex = &mutex;
	}
	pthread_t threads[thread_number];
	for (int i = 0; i < thread_number; i++) {
		pthread_create(&threads[i], NULL, file_copy, &data[i]);
	}
	for (int i = 0; i < 5; i++) {
		pthread_join(threads[i], NULL);
	}*/
	return 0;
}
